// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: VideoSplit.proto

#include "VideoSplit.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

extern PROTOBUF_INTERNAL_EXPORT_VideoSplitDefine_2eproto ::google::protobuf::internal::SCCInfo<0> scc_info_CameraInfo_VideoSplitDefine_2eproto;
class AddCameraInfosDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<AddCameraInfos> _instance;
} _AddCameraInfos_default_instance_;
static void InitDefaultsAddCameraInfos_VideoSplit_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::_AddCameraInfos_default_instance_;
    new (ptr) ::AddCameraInfos();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::AddCameraInfos::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<1> scc_info_AddCameraInfos_VideoSplit_2eproto =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 1, InitDefaultsAddCameraInfos_VideoSplit_2eproto}, {
      &scc_info_CameraInfo_VideoSplitDefine_2eproto.base,}};


// ===================================================================

void AddCameraInfos::InitAsDefaultInstance() {
}
class AddCameraInfos::HasBitSetters {
 public:
};

void AddCameraInfos::clear_camera_infos() {
  camera_infos_.Clear();
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int AddCameraInfos::kCameraInfosFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

AddCameraInfos::AddCameraInfos()
  : ::google::protobuf::MessageLite(), _internal_metadata_(nullptr) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:AddCameraInfos)
}
AddCameraInfos::AddCameraInfos(const AddCameraInfos& from)
  : ::google::protobuf::MessageLite(),
      _internal_metadata_(nullptr),
      _has_bits_(from._has_bits_),
      camera_infos_(from.camera_infos_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:AddCameraInfos)
}

void AddCameraInfos::SharedCtor() {
  ::google::protobuf::internal::InitSCC(
      &scc_info_AddCameraInfos_VideoSplit_2eproto.base);
}

AddCameraInfos::~AddCameraInfos() {
  // @@protoc_insertion_point(destructor:AddCameraInfos)
  SharedDtor();
}

void AddCameraInfos::SharedDtor() {
}

void AddCameraInfos::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const AddCameraInfos& AddCameraInfos::default_instance() {
  ::google::protobuf::internal::InitSCC(&::scc_info_AddCameraInfos_VideoSplit_2eproto.base);
  return *internal_default_instance();
}


void AddCameraInfos::Clear() {
// @@protoc_insertion_point(message_clear_start:AddCameraInfos)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  camera_infos_.Clear();
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

#if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
const char* AddCameraInfos::_InternalParse(const char* begin, const char* end, void* object,
                  ::google::protobuf::internal::ParseContext* ctx) {
  auto msg = static_cast<AddCameraInfos*>(object);
  ::google::protobuf::int32 size; (void)size;
  int depth; (void)depth;
  ::google::protobuf::uint32 tag;
  ::google::protobuf::internal::ParseFunc parser_till_end; (void)parser_till_end;
  auto ptr = begin;
  while (ptr < end) {
    ptr = ::google::protobuf::io::Parse32(ptr, &tag);
    GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
    switch (tag >> 3) {
      // repeated .CameraInfo camera_infos = 1;
      case 1: {
        if (static_cast<::google::protobuf::uint8>(tag) != 10) goto handle_unusual;
        do {
          ptr = ::google::protobuf::io::ReadSize(ptr, &size);
          GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
          parser_till_end = ::CameraInfo::_InternalParse;
          object = msg->add_camera_infos();
          if (size > end - ptr) goto len_delim_till_end;
          ptr += size;
          GOOGLE_PROTOBUF_PARSER_ASSERT(ctx->ParseExactRange(
              {parser_till_end, object}, ptr - size, ptr));
          if (ptr >= end) break;
        } while ((::google::protobuf::io::UnalignedLoad<::google::protobuf::uint64>(ptr) & 255) == 10 && (ptr += 1));
        break;
      }
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->EndGroup(tag);
          return ptr;
        }
        auto res = UnknownFieldParse(tag, {_InternalParse, msg},
          ptr, end, msg->_internal_metadata_.mutable_unknown_fields(), ctx);
        ptr = res.first;
        GOOGLE_PROTOBUF_PARSER_ASSERT(ptr != nullptr);
        if (res.second) return ptr;
      }
    }  // switch
  }  // while
  return ptr;
len_delim_till_end:
  return ctx->StoreAndTailCall(ptr, end, {_InternalParse, msg},
                               {parser_till_end, object}, size);
}
#else  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
bool AddCameraInfos::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!PROTOBUF_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  ::google::protobuf::internal::LiteUnknownFieldSetter unknown_fields_setter(
      &_internal_metadata_);
  ::google::protobuf::io::StringOutputStream unknown_fields_output(
      unknown_fields_setter.buffer());
  ::google::protobuf::io::CodedOutputStream unknown_fields_stream(
      &unknown_fields_output, false);
  // @@protoc_insertion_point(parse_start:AddCameraInfos)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // repeated .CameraInfo camera_infos = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) == (10 & 0xFF)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessage(
                input, add_camera_infos()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(
            input, tag, &unknown_fields_stream));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:AddCameraInfos)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:AddCameraInfos)
  return false;
#undef DO_
}
#endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER

void AddCameraInfos::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:AddCameraInfos)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated .CameraInfo camera_infos = 1;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->camera_infos_size()); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessage(
      1,
      this->camera_infos(static_cast<int>(i)),
      output);
  }

  output->WriteRaw(_internal_metadata_.unknown_fields().data(),
                   static_cast<int>(_internal_metadata_.unknown_fields().size()));
  // @@protoc_insertion_point(serialize_end:AddCameraInfos)
}

size_t AddCameraInfos::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:AddCameraInfos)
  size_t total_size = 0;

  total_size += _internal_metadata_.unknown_fields().size();

  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .CameraInfo camera_infos = 1;
  {
    unsigned int count = static_cast<unsigned int>(this->camera_infos_size());
    total_size += 1UL * count;
    for (unsigned int i = 0; i < count; i++) {
      total_size +=
        ::google::protobuf::internal::WireFormatLite::MessageSize(
          this->camera_infos(static_cast<int>(i)));
    }
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void AddCameraInfos::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const AddCameraInfos*>(&from));
}

void AddCameraInfos::MergeFrom(const AddCameraInfos& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:AddCameraInfos)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  camera_infos_.MergeFrom(from.camera_infos_);
}

void AddCameraInfos::CopyFrom(const AddCameraInfos& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:AddCameraInfos)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool AddCameraInfos::IsInitialized() const {
  return true;
}

void AddCameraInfos::Swap(AddCameraInfos* other) {
  if (other == this) return;
  InternalSwap(other);
}
void AddCameraInfos::InternalSwap(AddCameraInfos* other) {
  using std::swap;
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  CastToBase(&camera_infos_)->InternalSwap(CastToBase(&other->camera_infos_));
}

::std::string AddCameraInfos::GetTypeName() const {
  return "AddCameraInfos";
}


// @@protoc_insertion_point(namespace_scope)
namespace google {
namespace protobuf {
template<> PROTOBUF_NOINLINE ::AddCameraInfos* Arena::CreateMaybeMessage< ::AddCameraInfos >(Arena* arena) {
  return Arena::CreateInternal< ::AddCameraInfos >(arena);
}
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
