#include "image.h"
#include "vm_spec.h"

#define FT_MAGIC 0xF00BA125
#define FT_IMAGE_VERSION 0x00000100

struct ImageHeader
{
  uint32_t magic, version;
  uint32_t region_size_bytes, flags;
  uint64_t next_alloc;
  uint32_t config_size_pairs, config_capacity_pairs;
  uint32_t config[1][2];
};

enum ImageConfigOption
{
  ICO_SpecialObjects,
  ICO_Entrypoint,
  ICO_EndOfConfig,

  ICO__count
};

Image::Image(std::size_t image_size)
: Memory(mmap(nullptr, image_size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0), image_size, 0)
{
  int config_capacity = 16;
  next_alloc = sizeof(ImageHeader) + sizeof(uint32_t[2]) * config_capacity;

  ImageHeader* header = (ImageHeader*)region_start;
  header->config_size_pairs = 0;
  header->config_capacity_pairs = config_capacity;
  header->config[0][0] = ICO_EndOfConfig;

  update_header();
}

Image::Image(const char* path)
: Memory(nullptr, 0, 0)
{
  switch(load(path))
  {
  case ILR_OK:
    break;
  case ILR_ErrorOpeningFile:
    throw std::runtime_error("Could not open image file");
  case ILR_ErrorMMapFailed:
    throw std::runtime_error("Could not map image file");
  case ILR_ErrorFileSizeMismatch:
    throw std::runtime_error("Image file has invalid region size");
  }
}

Image::~Image()
{
  munmap(this->region_start, this->region_size);
}

void Image::replace_data(void* data, std::size_t size)
{
  munmap(region_start, region_size);
  region_start = data;
  region_size = size;

  special_objects = nullptr;

  ImageHeader* header = (ImageHeader*)region_start;
  for(unsigned int i = 0; i < header->config_size_pairs; ++i)
  {
    const auto& pair = header->config[i];
    switch(pair[0])
    {
    case ICO_SpecialObjects:
      if(!special_objects)
      {
        special_objects = (object_array)ptr(pair[1]);
      }
      break;

    default:
      break;
    }
  }
}

void Image::update_header()
{
  ImageHeader* header = (ImageHeader*)region_start;
  header->magic = FT_MAGIC;
  header->version = FT_IMAGE_VERSION;
  header->region_size_bytes = region_size;
  header->flags = 0;
  header->next_alloc = (uint64_t)next_alloc;

  int special_objects_written = 0;
  for(unsigned int i = 0; i < header->config_size_pairs; ++i)
  {
    auto& pair = header->config[i];
    switch(pair[0])
    {
    case ICO_SpecialObjects:
      pair[1] = offset(special_objects);
      ++special_objects_written;
      break;

    default:
      break;
    }
  }

  if(!special_objects_written)
  {
    if(header->config_size_pairs == header->config_capacity_pairs)
    {
      std::cerr << "cannot write special objects" << std::endl;
      throw TODO{};
    }
    auto& pair = header->config[ header->config_size_pairs++ ];
    pair[0] = ICO_SpecialObjects;
    pair[1] = offset(special_objects);
  }
}

void Image::add_entrypoint(oop method)
{
  ImageHeader* header = (ImageHeader*)region_start;
  if(header->config_size_pairs == header->config_capacity_pairs)
  {
    std::cerr << "cannot add entrypoint" << std::endl;
    throw TODO{};
  }
  auto& pair = header->config[ header->config_size_pairs++ ];
  pair[0] = ICO_Entrypoint;
  pair[1] = (uint32_t)method;
}

void Image::each_entrypoint(Image::EntrypointCallback callback, void* arg) const
{
  ImageHeader* header = (ImageHeader*)region_start;
  for(unsigned int i = 0; i < header->config_size_pairs; ++i)
  {
    const auto& pair = header->config[i];
    if(pair[0] == ICO_Entrypoint)
    {
      if(!callback((oop)pair[1], arg))
      {
        return;
      }
    }
  }
}


Image::ImageLoadResult Image::load(const char* filename)
{
  FILE* fp = fopen(filename, "r+");
  if(!fp)
  {
    return ILR_ErrorOpeningFile;
  }

  // get the size
  fseek(fp, 0, SEEK_END);
  auto file_size = (std::size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void* data = mmap(nullptr, file_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);
  fclose(fp);

  if(data == MAP_FAILED)
  {
    return ILR_ErrorMMapFailed;
  }

  if(!validate_header(data, file_size))
  {
    munmap(data, file_size);
    return ILR_ErrorFileSizeMismatch;
  }

  replace_data(data, file_size);
  return ILR_OK;
}

void Image::save(const char* filename)
{
  if(!region_size)
  {
    throw TODO{};
  }
  FILE* fp = fopen(filename, "w+");
  if(!fp)
  {
    std::cerr << "failed to open file for writing '" << filename << "' err= '" << strerror(errno) << "'" << std::endl;
    return;
  }

  update_header();
  if(fwrite(region_start, region_size, 1, fp) != 1)
  {
    std::cerr << "failed to write image to file, size= " << region_size << ", feof= " << feof(fp) << ", ferror= " << ferror(fp) << std::endl;
  }
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
  special_objects[soid_arrayVt] = offset(array_primitive_vt);

  vtable_object* integer_vt = vtable_object::make(64, 1, vtable_vt, *this);
  special_objects[soid_integerVt] = offset(integer_vt);

  vtable_object* globals_vt = vtable_object::make(128, 0, vtable_vt, *this);
  void* globals = alloc(globals_vt, 0);

  add_slot(lobby_vt, "Globals", vts_static_parent, offset(globals));

  add_slot(globals_vt, "Lobby", vts_static, offset(lobby));
  add_slot(globals_vt, "VTableVT", vts_static, offset(vtable_vt));
  add_slot(globals_vt, "PrimitiveArrayVT", vts_static, offset(array_primitive_vt));
  add_slot(globals_vt, "PrimitiveStringVT", vts_static, offset(string_primitive_vt));
  add_slot(globals_vt, "IntegerVT", vts_static, offset(integer_vt));

  // add typeName slots
  add_slot(vtable_vt, "typeName", vts_static, offset(intern("VTable")));
  add_slot(string_primitive_vt, "typeName", vts_static, offset(intern("String")));
  add_slot(array_primitive_vt, "typeName", vts_static, offset(intern("Array")));
  add_slot(lobby_vt, "typeName", vts_static, offset(intern("Lobby")));
  add_slot(globals_vt, "typeName", vts_static, offset(intern("Globals")));
  add_slot(integer_vt, "typeName", vts_static, offset(intern("Integer")));

  // base methods
  vtable_object* primitive_method_vt = vtable_object::make(64, 0, vtable_vt, *this);
  add_slot(primitive_method_vt, "typeName", vts_static, offset(intern("PrimitiveMethod")));

  auto make_prim_method = [&](primitive_id_t prim)
  {
    object_slots bytecode_obj = (object_slots)alloc_words(primitive_method_vt, VMBS__count);
    bytecode_obj[VMBS_PrimitiveProxyIndex] = int_to_oop(prim);

    vtable_object* method_vt = vtable_object::make(4, 0, vtable_vt, *this);
    method_vt->set_bytecode(offset(bytecode_obj));

    return method_vt->allocate_instance(*this);
  };

  // integerVt
  {
    void* integer_plus_ = make_prim_method(pid_Integer_plus_);
    add_slot(integer_vt, "+", vts_static, offset(integer_plus_));
    add_slot(integer_vt, "plus:", vts_static, offset(integer_plus_));

    void* integer_minus_ = make_prim_method(pid_Integer_minus_);
    add_slot(integer_vt, "-", vts_static, offset(integer_minus_));
    add_slot(integer_vt, "minus:", vts_static, offset(integer_minus_));

    void* integer_multiply_ = make_prim_method(pid_Integer_multiply_);
    add_slot(integer_vt, "*", vts_static, offset(integer_multiply_));
    add_slot(integer_vt, "multiply:", vts_static, offset(integer_multiply_));

    void* integer_divide_ = make_prim_method(pid_Integer_divide_);
    add_slot(integer_vt, "/", vts_static, offset(integer_divide_));
    add_slot(integer_vt, "divide:", vts_static, offset(integer_divide_));

    void* integer_print = make_prim_method(pid_Integer_print);
    add_slot(integer_vt, "print", vts_static, offset(integer_print));
  }
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

Image::add_slot_result_t Image::add_slot(vtable_object* vtable, const char* slot_name, vtable_slot_flags flags, oop value)
{
  return vtable->add_slot(*this, slot_name, flags, value);
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
