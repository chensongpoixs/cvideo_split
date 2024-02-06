// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: VideoSplit.proto

#ifndef PROTOBUF_INCLUDED_VideoSplit_2eproto
#define PROTOBUF_INCLUDED_VideoSplit_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3007000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3007000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include "VideoSplitDefine.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_VideoSplit_2eproto

// Internal implementation detail -- do not use these members.
struct TableStruct_VideoSplit_2eproto {
  static const ::google::protobuf::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::google::protobuf::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
class AddCameraInfos;
class AddCameraInfosDefaultTypeInternal;
extern AddCameraInfosDefaultTypeInternal _AddCameraInfos_default_instance_;
namespace google {
namespace protobuf {
template<> ::AddCameraInfos* Arena::CreateMaybeMessage<::AddCameraInfos>(Arena*);
}  // namespace protobuf
}  // namespace google

// ===================================================================

class AddCameraInfos final :
    public ::google::protobuf::MessageLite /* @@protoc_insertion_point(class_definition:AddCameraInfos) */ {
 public:
  AddCameraInfos();
  virtual ~AddCameraInfos();

  AddCameraInfos(const AddCameraInfos& from);

  inline AddCameraInfos& operator=(const AddCameraInfos& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  AddCameraInfos(AddCameraInfos&& from) noexcept
    : AddCameraInfos() {
    *this = ::std::move(from);
  }

  inline AddCameraInfos& operator=(AddCameraInfos&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const AddCameraInfos& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const AddCameraInfos* internal_default_instance() {
    return reinterpret_cast<const AddCameraInfos*>(
               &_AddCameraInfos_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(AddCameraInfos* other);
  friend void swap(AddCameraInfos& a, AddCameraInfos& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline AddCameraInfos* New() const final {
    return CreateMaybeMessage<AddCameraInfos>(nullptr);
  }

  AddCameraInfos* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<AddCameraInfos>(arena);
  }
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from)
    final;
  void CopyFrom(const AddCameraInfos& from);
  void MergeFrom(const AddCameraInfos& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  static const char* _InternalParse(const char* begin, const char* end, void* object, ::google::protobuf::internal::ParseContext* ctx);
  ::google::protobuf::internal::ParseFunc _ParseFunc() const final { return _InternalParse; }
  #else
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  void DiscardUnknownFields();
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(AddCameraInfos* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .CameraInfo camera_infos = 1;
  int camera_infos_size() const;
  void clear_camera_infos();
  static const int kCameraInfosFieldNumber = 1;
  ::CameraInfo* mutable_camera_infos(int index);
  ::google::protobuf::RepeatedPtrField< ::CameraInfo >*
      mutable_camera_infos();
  const ::CameraInfo& camera_infos(int index) const;
  ::CameraInfo* add_camera_infos();
  const ::google::protobuf::RepeatedPtrField< ::CameraInfo >&
      camera_infos() const;

  // @@protoc_insertion_point(class_scope:AddCameraInfos)
 private:
  class HasBitSetters;

  ::google::protobuf::internal::InternalMetadataWithArenaLite _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  ::google::protobuf::RepeatedPtrField< ::CameraInfo > camera_infos_;
  friend struct ::TableStruct_VideoSplit_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// AddCameraInfos

// repeated .CameraInfo camera_infos = 1;
inline int AddCameraInfos::camera_infos_size() const {
  return camera_infos_.size();
}
inline ::CameraInfo* AddCameraInfos::mutable_camera_infos(int index) {
  // @@protoc_insertion_point(field_mutable:AddCameraInfos.camera_infos)
  return camera_infos_.Mutable(index);
}
inline ::google::protobuf::RepeatedPtrField< ::CameraInfo >*
AddCameraInfos::mutable_camera_infos() {
  // @@protoc_insertion_point(field_mutable_list:AddCameraInfos.camera_infos)
  return &camera_infos_;
}
inline const ::CameraInfo& AddCameraInfos::camera_infos(int index) const {
  // @@protoc_insertion_point(field_get:AddCameraInfos.camera_infos)
  return camera_infos_.Get(index);
}
inline ::CameraInfo* AddCameraInfos::add_camera_infos() {
  // @@protoc_insertion_point(field_add:AddCameraInfos.camera_infos)
  return camera_infos_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::CameraInfo >&
AddCameraInfos::camera_infos() const {
  // @@protoc_insertion_point(field_list:AddCameraInfos.camera_infos)
  return camera_infos_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // PROTOBUF_INCLUDED_VideoSplit_2eproto
