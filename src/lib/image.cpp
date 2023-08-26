#include "image.h"

#define FT_MAGIC 0xF00BA125
#define FT_IMAGE_VERSION 0x00000100

struct ImageHeader
{
  uint32_t magic, version;
  uint32_t region_size_bytes, flags;
  uint64_t next_alloc, entrypoint, config_size_pairs;
  uint32_t config[0];
};

Image::Image(std::size_t image_size)
: Memory(mmap(nullptr, image_size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0), image_size, 0)
{
  ImageHeader* header = (ImageHeader*)region_start;
  header->region_size_bytes = image_size;
  header->flags = 0;
  next_alloc = (image_offset_t)header->next_alloc;
  if(!next_alloc)
  {
    next_alloc = header->next_alloc = sizeof(ImageHeader);
  }
}

Image::~Image()
{
  munmap(this->region_start, this->region_size);
}

void Image::replace_data(void* data, std::size_t size)
{
  munmap(this->region_start, this->region_size);
  this->region_start = data;
  this->region_size = size;
}

void Image::update_header()
{
  ImageHeader* header = (ImageHeader*)region_start;
  header->magic = FT_MAGIC;
  header->version = FT_IMAGE_VERSION;
  header->region_size_bytes = region_size;
  header->flags = 0;
  header->next_alloc = (uint64_t)next_alloc;
  header->config_size_pairs = 0;
}


void Image::load(const char* filename)
{
  FILE* fp = fopen(filename, "rw");
  if(!fp)
  {
    std::cerr << "failed to open file for reading '" << filename << "'" << std::endl;
    return;
  }

  // get the size
  fseek(fp, 0, SEEK_END);
  auto file_size = (uint32_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  int file_id = fileno(fp);
  void* data = mmap(nullptr, file_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, file_id, 0);
  fclose(fp);

  if(!data)
  {
    std::cerr << "failed to mmap file" << std::endl;
    return;
  }

  if(!validate_header(data, file_size))
  {
    munmap(data, file_size);
    std::cerr << "invalid image" << std::endl;
    return;
  }

  replace_data(data, file_size);
}

void Image::save(const char* filename)
{
  FILE* fp = fopen(filename, "w+");
  if(!fp)
  {
    std::cerr << "failed to open file for writing '" << filename << "'" << std::endl;
    return;
  }

  update_header();
  fwrite(region_start, region_size, 1, fp);
  fclose(fp);
}

void Image::set_vtable(void* object, vtable_object* new_vt)
{
  // *((void**)object - 1) = new_vt;
  ((object_array)object)[-1] = offset(new_vt);
}

bool Image::validate_header(void* region_ptr, std::size_t region_size_bytes) const
{
  auto header = (ImageHeader*)region_ptr;
  if(header->region_size_bytes != region_size_bytes
    || header->magic != FT_MAGIC
    || header->version != FT_IMAGE_VERSION)
  {
    return false;
  }
  return true;
}

void Image::bootstrap()
{
  // vtable vtable is used by 99% of vtables
  vtable_object* vtable_vt = vtable_object::make(32, 1, nullptr, *this);
  set_vtable(vtable_vt, vtable_vt);

  // "defaultBehavior" object
  vtable_object* root_vt = vtable_object::make(32, 0, vtable_vt, *this);
  void* root_object = alloc(root_vt, 0);

  vtable_vt->static_parents_begin()[0] = offset(root_object);

  // string primitive vt
  vtable_object* string_primitive_vt = vtable_object::make(32, 1, vtable_vt, *this);
  string_primitive_vt->static_parents_begin()[0] = offset(root_object);

  // array primitive vt
  vtable_object* array_primitive_vt = vtable_object::make(32, 1, vtable_vt, *this);
  special_objects = (object_array)alloc_words(array_primitive_vt, soid__count);
  special_objects[soid_symbolVt] = offset(string_primitive_vt);
  special_objects[soid_symbolMapVt] = offset(vtable_object::make(128, 0, vtable_vt, *this));
  // now strings can be interned for slots

  vtable_object* lobby_vt = vtable_object::make(128, 1, vtable_vt, *this);

  // holds a map with primitive info for the vm
  vtable_object* primitive_map_vt = vtable_object::make(pid__count, 0, vtable_vt, *this);

  void* lobby = alloc(lobby_vt, 0);

  special_objects[soid_vtableVt] = offset(vtable_vt);
  special_objects[soid_primitiveMap] = offset(alloc(primitive_map_vt, 0));
  special_objects[soid_lobby] = offset(lobby);

  vtable_object* globals_vt = vtable_object::make(128, 0, vtable_vt, *this);
  void* globals = alloc(globals_vt, 0);

  add_slot(lobby_vt, "Globals", vts_static_parent, globals);

  add_slot(globals_vt, "Lobby", vts_static, lobby);
  add_slot(globals_vt, "VTableVT", vts_static, vtable_vt);
  add_slot(globals_vt, "PrimitiveArrayVT", vts_static, array_primitive_vt);
  add_slot(globals_vt, "PrimitiveStringVT", vts_static, string_primitive_vt);

  // add typeName slots
  add_slot(vtable_vt, "typeName", vts_static, intern("VTable"));
  add_slot(string_primitive_vt, "typeName", vts_static, intern("String"));
  add_slot(array_primitive_vt, "typeName", vts_static, intern("Array"));
  add_slot(lobby_vt, "typeName", vts_static, intern("Lobby"));
  add_slot(globals_vt, "typeName", vts_static, intern("Globals"));
}

inline unsigned int djb2(const char* str)
{
  unsigned long hash = 5381;
  for(int c; (c = *str++); )
  {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

unsigned int Image::hash_symbol(string_ref symbol)
{
  return djb2((const char*)symbol);
}

Image::add_slot_result_t Image::add_slot(vtable_object* vtable, const char* slot_name, vtable_slot_flags flags, void* value_object)
{
  if(vtable->slot_count == vtable->slot_capacity)
  {
    return asr_error_too_many_slots;
  }

  flags = (vtable_slot_flags)(flags & vts__mask);

  string_ref symbol = intern(slot_name);
  oop symbol_offset = offset(symbol);
  const unsigned int slot_mask = vtable->slot_capacity - 1;
  auto hash = hash_symbol(symbol);
  unsigned int index = hash & slot_mask;
  vtable_slot* slot = vtable->slots_begin() + index;
  while(! slot->empty())
  {
    if(slot->key() == symbol_offset)
    {
      return asr_error_slot_exists;
    }
    index = (index + 1) & slot_mask;
    slot = vtable->slots_begin() + index;
  }

  oop value = offset(value_object);

  switch(flags)
  {
  case vts_static_parent:
  {
    // static parent, validate vtable can fit more static parents
    int static_parent_index = -1;
    for(std::size_t i = 0; i < vtable->static_parent_count; ++i)
    {
      if(! vtable->static_parents_begin()[i])
      {
        static_parent_index = (int)i;
        break;
      }
    }
    if(static_parent_index == -1)
    {
      // no room for static parent
      return asr_error_too_many_static_parents;
    }
    vtable->static_parents_begin()[ static_parent_index ] = value;
    *slot = vtable_slot(flags, symbol_offset, int_to_oop(static_parent_index));

  } break;

  case vts_static:
    // static data slot
    *slot = vtable_slot(flags, symbol_offset, value);
    break;

  case vts_parent:

    if(vtable->instance_size_words > vtable->parent_count)
    {
      // non-parent slots already defined, cannot define parent slot
      return asr_error_reorder_parents;
    }
    ++vtable->parent_count;
    // fall-through: these are normal parent slots

  case vts_data:
  {
    // instance data slot
    auto instance_index = vtable->instance_size_words ++;
    *slot = vtable_slot(flags, symbol_offset, int_to_oop(instance_index));

  } break;


  default:
    return asr_error_invalid_type;

  }

  ++ vtable->slot_count;
  return asr_ok;

}

string_ref Image::intern(const char* symbol)
{
  auto hash = djb2(symbol);
  auto symbol_map_vt = (vtable_object*)ptr(special_object(soid_symbolMapVt));
  const unsigned int slot_mask = symbol_map_vt->slot_capacity - 1;
  unsigned int index = hash & slot_mask;
  vtable_slot* slot = symbol_map_vt->slots_begin() + index;
  for(; !slot->empty(); slot = symbol_map_vt->slots_begin() + (++index & slot_mask))
  {
    if(!std::strcmp((string_ref)ptr(slot->key()), symbol))
    {
      return (string_ref)ptr(slot->value());
    }
  }

  if(symbol_map_vt->slot_count == symbol_map_vt->slot_capacity)
  {
    std::cerr << "TODO: symbol map VT capacity reached" << std::endl;
    throw TODO{};
  }

  vtable_object* symbolVt = (vtable_object*)ptr(special_object(soid_symbolVt));
  auto len = strlen(symbol);
  char* new_symbol = (char*)alloc(symbolVt, len + 1);
  strcpy(new_symbol, symbol);
  new_symbol[len] = 0;

  // manual add_slot here
  *slot = vtable_slot(vts_static, offset(new_symbol), offset(new_symbol));
  ++ symbol_map_vt->slot_count;

  return new_symbol;
}

oop Image::special_object(std::size_t index) const
{
  return special_objects[index];
}

