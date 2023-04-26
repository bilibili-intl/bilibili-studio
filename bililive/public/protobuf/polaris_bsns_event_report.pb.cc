// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: polaris_bsns_event_report.proto

#include "polaris_bsns_event_report.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
constexpr Event_MetaEntry_DoNotUse::Event_MetaEntry_DoNotUse(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized){}
struct Event_MetaEntry_DoNotUseDefaultTypeInternal {
  constexpr Event_MetaEntry_DoNotUseDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~Event_MetaEntry_DoNotUseDefaultTypeInternal() {}
  union {
    Event_MetaEntry_DoNotUse _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT Event_MetaEntry_DoNotUseDefaultTypeInternal _Event_MetaEntry_DoNotUse_default_instance_;
constexpr Event::Event(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : meta_(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{})
  , logid_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , eventid_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , body_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string){}
struct EventDefaultTypeInternal {
  constexpr EventDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~EventDefaultTypeInternal() {}
  union {
    Event _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT EventDefaultTypeInternal _Event_default_instance_;
constexpr EventList::EventList(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : events_(){}
struct EventListDefaultTypeInternal {
  constexpr EventListDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~EventListDefaultTypeInternal() {}
  union {
    EventList _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT EventListDefaultTypeInternal _EventList_default_instance_;
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_polaris_5fbsns_5fevent_5freport_2eproto[3];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_polaris_5fbsns_5fevent_5freport_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_polaris_5fbsns_5fevent_5freport_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_polaris_5fbsns_5fevent_5freport_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  PROTOBUF_FIELD_OFFSET(::Event_MetaEntry_DoNotUse, _has_bits_),
  PROTOBUF_FIELD_OFFSET(::Event_MetaEntry_DoNotUse, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::Event_MetaEntry_DoNotUse, key_),
  PROTOBUF_FIELD_OFFSET(::Event_MetaEntry_DoNotUse, value_),
  0,
  1,
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::Event, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::Event, logid_),
  PROTOBUF_FIELD_OFFSET(::Event, eventid_),
  PROTOBUF_FIELD_OFFSET(::Event, body_),
  PROTOBUF_FIELD_OFFSET(::Event, meta_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::EventList, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::EventList, events_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 7, sizeof(::Event_MetaEntry_DoNotUse)},
  { 9, -1, sizeof(::Event)},
  { 18, -1, sizeof(::EventList)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::_Event_MetaEntry_DoNotUse_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::_Event_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::_EventList_default_instance_),
};

const char descriptor_table_protodef_polaris_5fbsns_5fevent_5freport_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\037polaris_bsns_event_report.proto\"\202\001\n\005Ev"
  "ent\022\r\n\005logId\030\001 \001(\t\022\017\n\007eventId\030\002 \001(\t\022\014\n\004b"
  "ody\030\003 \001(\014\022\036\n\004meta\030\004 \003(\0132\020.Event.MetaEntr"
  "y\032+\n\tMetaEntry\022\013\n\003key\030\001 \001(\t\022\r\n\005value\030\002 \001"
  "(\t:\0028\001\"#\n\tEventList\022\026\n\006events\030\001 \003(\0132\006.Ev"
  "entb\006proto3"
  ;
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto = {
  false, false, 211, descriptor_table_protodef_polaris_5fbsns_5fevent_5freport_2eproto, "polaris_bsns_event_report.proto", 
  &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_once, nullptr, 0, 3,
  schemas, file_default_instances, TableStruct_polaris_5fbsns_5fevent_5freport_2eproto::offsets,
  file_level_metadata_polaris_5fbsns_5fevent_5freport_2eproto, file_level_enum_descriptors_polaris_5fbsns_5fevent_5freport_2eproto, file_level_service_descriptors_polaris_5fbsns_5fevent_5freport_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_getter() {
  return &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_polaris_5fbsns_5fevent_5freport_2eproto(&descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto);

// ===================================================================

Event_MetaEntry_DoNotUse::Event_MetaEntry_DoNotUse() {}
Event_MetaEntry_DoNotUse::Event_MetaEntry_DoNotUse(::PROTOBUF_NAMESPACE_ID::Arena* arena)
    : SuperType(arena) {}
void Event_MetaEntry_DoNotUse::MergeFrom(const Event_MetaEntry_DoNotUse& other) {
  MergeFromInternal(other);
}
::PROTOBUF_NAMESPACE_ID::Metadata Event_MetaEntry_DoNotUse::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_getter, &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_once,
      file_level_metadata_polaris_5fbsns_5fevent_5freport_2eproto[0]);
}

// ===================================================================

class Event::_Internal {
 public:
};

Event::Event(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned),
  meta_(arena) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:Event)
}
Event::Event(const Event& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  meta_.MergeFrom(from.meta_);
  logid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_logid().empty()) {
    logid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_logid(), 
      GetArenaForAllocation());
  }
  eventid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_eventid().empty()) {
    eventid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_eventid(), 
      GetArenaForAllocation());
  }
  body_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_body().empty()) {
    body_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_body(), 
      GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:Event)
}

inline void Event::SharedCtor() {
logid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
eventid_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
body_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

Event::~Event() {
  // @@protoc_insertion_point(destructor:Event)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void Event::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  logid_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  eventid_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  body_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void Event::ArenaDtor(void* object) {
  Event* _this = reinterpret_cast< Event* >(object);
  (void)_this;
  _this->meta_. ~MapField();
}
inline void Event::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena) {
  if (arena != nullptr) {
    arena->OwnCustomDestructor(this, &Event::ArenaDtor);
  }
}
void Event::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void Event::Clear() {
// @@protoc_insertion_point(message_clear_start:Event)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  meta_.Clear();
  logid_.ClearToEmpty();
  eventid_.ClearToEmpty();
  body_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Event::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string logId = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_logid();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "Event.logId"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string eventId = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          auto str = _internal_mutable_eventid();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "Event.eventId"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // bytes body = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 26)) {
          auto str = _internal_mutable_body();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // map<string, string> meta = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 34)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(&meta_, ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<34>(ptr));
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag == 0) || ((tag & 7) == 4)) {
          CHK_(ptr);
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* Event::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:Event)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string logId = 1;
  if (!this->_internal_logid().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_logid().data(), static_cast<int>(this->_internal_logid().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "Event.logId");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_logid(), target);
  }

  // string eventId = 2;
  if (!this->_internal_eventid().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_eventid().data(), static_cast<int>(this->_internal_eventid().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "Event.eventId");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_eventid(), target);
  }

  // bytes body = 3;
  if (!this->_internal_body().empty()) {
    target = stream->WriteBytesMaybeAliased(
        3, this->_internal_body(), target);
  }

  // map<string, string> meta = 4;
  if (!this->_internal_meta().empty()) {
    typedef ::PROTOBUF_NAMESPACE_ID::Map< std::string, std::string >::const_pointer
        ConstPtr;
    typedef ConstPtr SortItem;
    typedef ::PROTOBUF_NAMESPACE_ID::internal::CompareByDerefFirst<SortItem> Less;
    struct Utf8Check {
      static void Check(ConstPtr p) {
        (void)p;
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
          p->first.data(), static_cast<int>(p->first.length()),
          ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
          "Event.MetaEntry.key");
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
          p->second.data(), static_cast<int>(p->second.length()),
          ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
          "Event.MetaEntry.value");
      }
    };

    if (stream->IsSerializationDeterministic() &&
        this->_internal_meta().size() > 1) {
      ::std::unique_ptr<SortItem[]> items(
          new SortItem[this->_internal_meta().size()]);
      typedef ::PROTOBUF_NAMESPACE_ID::Map< std::string, std::string >::size_type size_type;
      size_type n = 0;
      for (::PROTOBUF_NAMESPACE_ID::Map< std::string, std::string >::const_iterator
          it = this->_internal_meta().begin();
          it != this->_internal_meta().end(); ++it, ++n) {
        items[static_cast<ptrdiff_t>(n)] = SortItem(&*it);
      }
      ::std::sort(&items[0], &items[static_cast<ptrdiff_t>(n)], Less());
      for (size_type i = 0; i < n; i++) {
        target = Event_MetaEntry_DoNotUse::Funcs::InternalSerialize(4, items[static_cast<ptrdiff_t>(i)]->first, items[static_cast<ptrdiff_t>(i)]->second, target, stream);
        Utf8Check::Check(&(*items[static_cast<ptrdiff_t>(i)]));
      }
    } else {
      for (::PROTOBUF_NAMESPACE_ID::Map< std::string, std::string >::const_iterator
          it = this->_internal_meta().begin();
          it != this->_internal_meta().end(); ++it) {
        target = Event_MetaEntry_DoNotUse::Funcs::InternalSerialize(4, it->first, it->second, target, stream);
        Utf8Check::Check(&(*it));
      }
    }
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Event)
  return target;
}

size_t Event::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Event)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // map<string, string> meta = 4;
  total_size += 1 *
      ::PROTOBUF_NAMESPACE_ID::internal::FromIntSize(this->_internal_meta_size());
  for (::PROTOBUF_NAMESPACE_ID::Map< std::string, std::string >::const_iterator
      it = this->_internal_meta().begin();
      it != this->_internal_meta().end(); ++it) {
    total_size += Event_MetaEntry_DoNotUse::Funcs::ByteSizeLong(it->first, it->second);
  }

  // string logId = 1;
  if (!this->_internal_logid().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_logid());
  }

  // string eventId = 2;
  if (!this->_internal_eventid().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_eventid());
  }

  // bytes body = 3;
  if (!this->_internal_body().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::BytesSize(
        this->_internal_body());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Event::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    Event::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Event::GetClassData() const { return &_class_data_; }

void Event::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<Event *>(to)->MergeFrom(
      static_cast<const Event &>(from));
}


void Event::MergeFrom(const Event& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:Event)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  meta_.MergeFrom(from.meta_);
  if (!from._internal_logid().empty()) {
    _internal_set_logid(from._internal_logid());
  }
  if (!from._internal_eventid().empty()) {
    _internal_set_eventid(from._internal_eventid());
  }
  if (!from._internal_body().empty()) {
    _internal_set_body(from._internal_body());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Event::CopyFrom(const Event& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Event)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Event::IsInitialized() const {
  return true;
}

void Event::InternalSwap(Event* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  meta_.InternalSwap(&other->meta_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &logid_, GetArenaForAllocation(),
      &other->logid_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &eventid_, GetArenaForAllocation(),
      &other->eventid_, other->GetArenaForAllocation()
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &body_, GetArenaForAllocation(),
      &other->body_, other->GetArenaForAllocation()
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata Event::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_getter, &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_once,
      file_level_metadata_polaris_5fbsns_5fevent_5freport_2eproto[1]);
}

// ===================================================================

class EventList::_Internal {
 public:
};

EventList::EventList(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned),
  events_(arena) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:EventList)
}
EventList::EventList(const EventList& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      events_(from.events_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:EventList)
}

inline void EventList::SharedCtor() {
}

EventList::~EventList() {
  // @@protoc_insertion_point(destructor:EventList)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void EventList::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
}

void EventList::ArenaDtor(void* object) {
  EventList* _this = reinterpret_cast< EventList* >(object);
  (void)_this;
}
void EventList::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void EventList::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void EventList::Clear() {
// @@protoc_insertion_point(message_clear_start:EventList)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  events_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* EventList::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // repeated .Event events = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_events(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<10>(ptr));
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag == 0) || ((tag & 7) == 4)) {
          CHK_(ptr);
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* EventList::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:EventList)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated .Event events = 1;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->_internal_events_size()); i < n; i++) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(1, this->_internal_events(i), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:EventList)
  return target;
}

size_t EventList::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:EventList)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .Event events = 1;
  total_size += 1UL * this->_internal_events_size();
  for (const auto& msg : this->events_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData EventList::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    EventList::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*EventList::GetClassData() const { return &_class_data_; }

void EventList::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to,
                      const ::PROTOBUF_NAMESPACE_ID::Message&from) {
  static_cast<EventList *>(to)->MergeFrom(
      static_cast<const EventList &>(from));
}


void EventList::MergeFrom(const EventList& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:EventList)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  events_.MergeFrom(from.events_);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void EventList::CopyFrom(const EventList& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:EventList)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EventList::IsInitialized() const {
  return true;
}

void EventList::InternalSwap(EventList* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  events_.InternalSwap(&other->events_);
}

::PROTOBUF_NAMESPACE_ID::Metadata EventList::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_getter, &descriptor_table_polaris_5fbsns_5fevent_5freport_2eproto_once,
      file_level_metadata_polaris_5fbsns_5fevent_5freport_2eproto[2]);
}

// @@protoc_insertion_point(namespace_scope)
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::Event_MetaEntry_DoNotUse* Arena::CreateMaybeMessage< ::Event_MetaEntry_DoNotUse >(Arena* arena) {
  return Arena::CreateMessageInternal< ::Event_MetaEntry_DoNotUse >(arena);
}
template<> PROTOBUF_NOINLINE ::Event* Arena::CreateMaybeMessage< ::Event >(Arena* arena) {
  return Arena::CreateMessageInternal< ::Event >(arena);
}
template<> PROTOBUF_NOINLINE ::EventList* Arena::CreateMaybeMessage< ::EventList >(Arena* arena) {
  return Arena::CreateMessageInternal< ::EventList >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
