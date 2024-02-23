// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ErrorCode.proto

#ifndef PROTOBUF_INCLUDED_ErrorCode_2eproto
#define PROTOBUF_INCLUDED_ErrorCode_2eproto

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
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_util.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_ErrorCode_2eproto

// Internal implementation detail -- do not use these members.
struct TableStruct_ErrorCode_2eproto {
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
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

enum EErrorCode {
  EEC_Success = 0,
  EEC_Failed = 1,
  EEC_InvalidClientSessionID = 2,
  EEC_OtherDeviceLogin = 3,
  EEC_ClientConnectStatus = 4,
  EEC_MemAllocFailed = 5,
  EEC_DongleAuthDb = 300,
  EEC_DongleAuthNotAppid = 301,
  EEC_DongLeAuthLoadDefaultProject = 302,
  EEC_DongleAuthNotfindDeviceOnlineServer = 304,
  EEC_DongleAuthExpireTimestamp = 305,
  EEC_DongleAuthNotStartTimestamp = 306,
  EEC_DongleAuthServiceModifyComputerTime = 307,
  EEC_DongleAuthNotfindPOItag = 308,
  EEC_DongleAuthNotfindtag = 309,
  EEC_DongleAuthNotfindCameraChannel = 310,
  EEC_DongleAuthNotfindTagString = 311,
  EEC_DongleAuthcameraChannel = 312,
  EEC_DongleAuthPoiTagCameraCount = 313,
  EEC_DongleAuthConfigTable = 314,
  EEC_DongleAuthNotData = 315,
  EEC_DongleAuthServerTimstamp = 316,
  EEC_RenderCentralNotFindDir = 500,
  EEC_AuthRenderDb = 900,
  EEC_AuthRenderNotAuth = 901,
  EEC_AuthRenderAuthPass = 902,
  EEC_AuthRenderExpireTimestamp = 903,
  EEC_AuthRenderDiffDevice = 904,
  EEC_AuthRenderOtherAuthLogin = 905,
  EEC_RenderServerLogin = 1000,
  EEC_SaveRenderServerDb = 1001
};
bool EErrorCode_IsValid(int value);
const EErrorCode EErrorCode_MIN = EEC_Success;
const EErrorCode EErrorCode_MAX = EEC_SaveRenderServerDb;
const int EErrorCode_ARRAYSIZE = EErrorCode_MAX + 1;

// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::EErrorCode> : ::std::true_type {};

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // PROTOBUF_INCLUDED_ErrorCode_2eproto