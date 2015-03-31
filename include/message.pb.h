// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: message.proto

#ifndef PROTOBUF_message_2eproto__INCLUDED
#define PROTOBUF_message_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_message_2eproto();
void protobuf_AssignDesc_message_2eproto();
void protobuf_ShutdownFile_message_2eproto();

class Request;
class Request_Field;
class Response;
class Response_Field;
class Response_Record;

// ===================================================================

class Request_Field : public ::google::protobuf::Message
{
public:
    Request_Field();
    virtual ~Request_Field();

    Request_Field(const Request_Field &from);

    inline Request_Field &operator=(const Request_Field &from)
    {
        CopyFrom(from);
        return *this;
    }

    inline const ::google::protobuf::UnknownFieldSet &unknown_fields() const
    {
        return _unknown_fields_;
    }

    inline ::google::protobuf::UnknownFieldSet *mutable_unknown_fields()
    {
        return &_unknown_fields_;
    }

    static const ::google::protobuf::Descriptor *descriptor();
    static const Request_Field &default_instance();

    void Swap(Request_Field *other);

    // implements Message ----------------------------------------------

    Request_Field *New() const;
    void CopyFrom(const ::google::protobuf::Message &from);
    void MergeFrom(const ::google::protobuf::Message &from);
    void CopyFrom(const Request_Field &from);
    void MergeFrom(const Request_Field &from);
    void Clear();
    bool IsInitialized() const;

    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream *input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream *output) const;
    ::google::protobuf::uint8 *SerializeWithCachedSizesToArray(::google::protobuf::uint8 *output) const;
    int GetCachedSize() const
    {
        return _cached_size_;
    }
private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;
public:

    ::google::protobuf::Metadata GetMetadata() const;

    // nested types ----------------------------------------------------

    // accessors -------------------------------------------------------

    // required string name = 1;
    inline bool has_name() const;
    inline void clear_name();
    static const int kNameFieldNumber = 1;
    inline const ::std::string &name() const;
    inline void set_name(const ::std::string &value);
    inline void set_name(const char *value);
    inline void set_name(const char *value, size_t size);
    inline ::std::string *mutable_name();
    inline ::std::string *release_name();

    // required string value = 2;
    inline bool has_value() const;
    inline void clear_value();
    static const int kValueFieldNumber = 2;
    inline const ::std::string &value() const;
    inline void set_value(const ::std::string &value);
    inline void set_value(const char *value);
    inline void set_value(const char *value, size_t size);
    inline ::std::string *mutable_value();
    inline ::std::string *release_value();

    // @@protoc_insertion_point(class_scope:Request.Field)
private:
    inline void set_has_name();
    inline void clear_has_name();
    inline void set_has_value();
    inline void clear_has_value();

    ::google::protobuf::UnknownFieldSet _unknown_fields_;

    ::std::string *name_;
    ::std::string *value_;

    mutable int _cached_size_;
    ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

    friend void  protobuf_AddDesc_message_2eproto();
    friend void protobuf_AssignDesc_message_2eproto();
    friend void protobuf_ShutdownFile_message_2eproto();

    void InitAsDefaultInstance();
    static Request_Field *default_instance_;
};
// -------------------------------------------------------------------

class Request : public ::google::protobuf::Message
{
public:
    Request();
    virtual ~Request();

    Request(const Request &from);

    inline Request &operator=(const Request &from)
    {
        CopyFrom(from);
        return *this;
    }

    inline const ::google::protobuf::UnknownFieldSet &unknown_fields() const
    {
        return _unknown_fields_;
    }

    inline ::google::protobuf::UnknownFieldSet *mutable_unknown_fields()
    {
        return &_unknown_fields_;
    }

    static const ::google::protobuf::Descriptor *descriptor();
    static const Request &default_instance();

    void Swap(Request *other);

    // implements Message ----------------------------------------------

    Request *New() const;
    void CopyFrom(const ::google::protobuf::Message &from);
    void MergeFrom(const ::google::protobuf::Message &from);
    void CopyFrom(const Request &from);
    void MergeFrom(const Request &from);
    void Clear();
    bool IsInitialized() const;

    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream *input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream *output) const;
    ::google::protobuf::uint8 *SerializeWithCachedSizesToArray(::google::protobuf::uint8 *output) const;
    int GetCachedSize() const
    {
        return _cached_size_;
    }
private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;
public:

    ::google::protobuf::Metadata GetMetadata() const;

    // nested types ----------------------------------------------------

    typedef Request_Field Field;

    // accessors -------------------------------------------------------

    // required string tablename = 1;
    inline bool has_tablename() const;
    inline void clear_tablename();
    static const int kTablenameFieldNumber = 1;
    inline const ::std::string &tablename() const;
    inline void set_tablename(const ::std::string &value);
    inline void set_tablename(const char *value);
    inline void set_tablename(const char *value, size_t size);
    inline ::std::string *mutable_tablename();
    inline ::std::string *release_tablename();

    // repeated .Request.Field fields = 2;
    inline int fields_size() const;
    inline void clear_fields();
    static const int kFieldsFieldNumber = 2;
    inline const ::Request_Field &fields(int index) const;
    inline ::Request_Field *mutable_fields(int index);
    inline ::Request_Field *add_fields();
    inline const ::google::protobuf::RepeatedPtrField< ::Request_Field > &
    fields() const;
    inline ::google::protobuf::RepeatedPtrField< ::Request_Field > *
    mutable_fields();

    // @@protoc_insertion_point(class_scope:Request)
private:
    inline void set_has_tablename();
    inline void clear_has_tablename();

    ::google::protobuf::UnknownFieldSet _unknown_fields_;

    ::std::string *tablename_;
    ::google::protobuf::RepeatedPtrField< ::Request_Field > fields_;

    mutable int _cached_size_;
    ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

    friend void  protobuf_AddDesc_message_2eproto();
    friend void protobuf_AssignDesc_message_2eproto();
    friend void protobuf_ShutdownFile_message_2eproto();

    void InitAsDefaultInstance();
    static Request *default_instance_;
};
// -------------------------------------------------------------------

class Response_Field : public ::google::protobuf::Message
{
public:
    Response_Field();
    virtual ~Response_Field();

    Response_Field(const Response_Field &from);

    inline Response_Field &operator=(const Response_Field &from)
    {
        CopyFrom(from);
        return *this;
    }

    inline const ::google::protobuf::UnknownFieldSet &unknown_fields() const
    {
        return _unknown_fields_;
    }

    inline ::google::protobuf::UnknownFieldSet *mutable_unknown_fields()
    {
        return &_unknown_fields_;
    }

    static const ::google::protobuf::Descriptor *descriptor();
    static const Response_Field &default_instance();

    void Swap(Response_Field *other);

    // implements Message ----------------------------------------------

    Response_Field *New() const;
    void CopyFrom(const ::google::protobuf::Message &from);
    void MergeFrom(const ::google::protobuf::Message &from);
    void CopyFrom(const Response_Field &from);
    void MergeFrom(const Response_Field &from);
    void Clear();
    bool IsInitialized() const;

    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream *input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream *output) const;
    ::google::protobuf::uint8 *SerializeWithCachedSizesToArray(::google::protobuf::uint8 *output) const;
    int GetCachedSize() const
    {
        return _cached_size_;
    }
private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;
public:

    ::google::protobuf::Metadata GetMetadata() const;

    // nested types ----------------------------------------------------

    // accessors -------------------------------------------------------

    // required string name = 1;
    inline bool has_name() const;
    inline void clear_name();
    static const int kNameFieldNumber = 1;
    inline const ::std::string &name() const;
    inline void set_name(const ::std::string &value);
    inline void set_name(const char *value);
    inline void set_name(const char *value, size_t size);
    inline ::std::string *mutable_name();
    inline ::std::string *release_name();

    // required string value = 2;
    inline bool has_value() const;
    inline void clear_value();
    static const int kValueFieldNumber = 2;
    inline const ::std::string &value() const;
    inline void set_value(const ::std::string &value);
    inline void set_value(const char *value);
    inline void set_value(const char *value, size_t size);
    inline ::std::string *mutable_value();
    inline ::std::string *release_value();

    // @@protoc_insertion_point(class_scope:Response.Field)
private:
    inline void set_has_name();
    inline void clear_has_name();
    inline void set_has_value();
    inline void clear_has_value();

    ::google::protobuf::UnknownFieldSet _unknown_fields_;

    ::std::string *name_;
    ::std::string *value_;

    mutable int _cached_size_;
    ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

    friend void  protobuf_AddDesc_message_2eproto();
    friend void protobuf_AssignDesc_message_2eproto();
    friend void protobuf_ShutdownFile_message_2eproto();

    void InitAsDefaultInstance();
    static Response_Field *default_instance_;
};
// -------------------------------------------------------------------

class Response_Record : public ::google::protobuf::Message
{
public:
    Response_Record();
    virtual ~Response_Record();

    Response_Record(const Response_Record &from);

    inline Response_Record &operator=(const Response_Record &from)
    {
        CopyFrom(from);
        return *this;
    }

    inline const ::google::protobuf::UnknownFieldSet &unknown_fields() const
    {
        return _unknown_fields_;
    }

    inline ::google::protobuf::UnknownFieldSet *mutable_unknown_fields()
    {
        return &_unknown_fields_;
    }

    static const ::google::protobuf::Descriptor *descriptor();
    static const Response_Record &default_instance();

    void Swap(Response_Record *other);

    // implements Message ----------------------------------------------

    Response_Record *New() const;
    void CopyFrom(const ::google::protobuf::Message &from);
    void MergeFrom(const ::google::protobuf::Message &from);
    void CopyFrom(const Response_Record &from);
    void MergeFrom(const Response_Record &from);
    void Clear();
    bool IsInitialized() const;

    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream *input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream *output) const;
    ::google::protobuf::uint8 *SerializeWithCachedSizesToArray(::google::protobuf::uint8 *output) const;
    int GetCachedSize() const
    {
        return _cached_size_;
    }
private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;
public:

    ::google::protobuf::Metadata GetMetadata() const;

    // nested types ----------------------------------------------------

    // accessors -------------------------------------------------------

    // repeated .Response.Field fields = 1;
    inline int fields_size() const;
    inline void clear_fields();
    static const int kFieldsFieldNumber = 1;
    inline const ::Response_Field &fields(int index) const;
    inline ::Response_Field *mutable_fields(int index);
    inline ::Response_Field *add_fields();
    inline const ::google::protobuf::RepeatedPtrField< ::Response_Field > &
    fields() const;
    inline ::google::protobuf::RepeatedPtrField< ::Response_Field > *
    mutable_fields();

    // @@protoc_insertion_point(class_scope:Response.Record)
private:

    ::google::protobuf::UnknownFieldSet _unknown_fields_;

    ::google::protobuf::RepeatedPtrField< ::Response_Field > fields_;

    mutable int _cached_size_;
    ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

    friend void  protobuf_AddDesc_message_2eproto();
    friend void protobuf_AssignDesc_message_2eproto();
    friend void protobuf_ShutdownFile_message_2eproto();

    void InitAsDefaultInstance();
    static Response_Record *default_instance_;
};
// -------------------------------------------------------------------

class Response : public ::google::protobuf::Message
{
public:
    Response();
    virtual ~Response();

    Response(const Response &from);

    inline Response &operator=(const Response &from)
    {
        CopyFrom(from);
        return *this;
    }

    inline const ::google::protobuf::UnknownFieldSet &unknown_fields() const
    {
        return _unknown_fields_;
    }

    inline ::google::protobuf::UnknownFieldSet *mutable_unknown_fields()
    {
        return &_unknown_fields_;
    }

    static const ::google::protobuf::Descriptor *descriptor();
    static const Response &default_instance();

    void Swap(Response *other);

    // implements Message ----------------------------------------------

    Response *New() const;
    void CopyFrom(const ::google::protobuf::Message &from);
    void MergeFrom(const ::google::protobuf::Message &from);
    void CopyFrom(const Response &from);
    void MergeFrom(const Response &from);
    void Clear();
    bool IsInitialized() const;

    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream *input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream *output) const;
    ::google::protobuf::uint8 *SerializeWithCachedSizesToArray(::google::protobuf::uint8 *output) const;
    int GetCachedSize() const
    {
        return _cached_size_;
    }
private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;
public:

    ::google::protobuf::Metadata GetMetadata() const;

    // nested types ----------------------------------------------------

    typedef Response_Field Field;
    typedef Response_Record Record;

    // accessors -------------------------------------------------------

    // required string tablename = 1;
    inline bool has_tablename() const;
    inline void clear_tablename();
    static const int kTablenameFieldNumber = 1;
    inline const ::std::string &tablename() const;
    inline void set_tablename(const ::std::string &value);
    inline void set_tablename(const char *value);
    inline void set_tablename(const char *value, size_t size);
    inline ::std::string *mutable_tablename();
    inline ::std::string *release_tablename();

    // repeated .Response.Record records = 2;
    inline int records_size() const;
    inline void clear_records();
    static const int kRecordsFieldNumber = 2;
    inline const ::Response_Record &records(int index) const;
    inline ::Response_Record *mutable_records(int index);
    inline ::Response_Record *add_records();
    inline const ::google::protobuf::RepeatedPtrField< ::Response_Record > &
    records() const;
    inline ::google::protobuf::RepeatedPtrField< ::Response_Record > *
    mutable_records();

    // @@protoc_insertion_point(class_scope:Response)
private:
    inline void set_has_tablename();
    inline void clear_has_tablename();

    ::google::protobuf::UnknownFieldSet _unknown_fields_;

    ::std::string *tablename_;
    ::google::protobuf::RepeatedPtrField< ::Response_Record > records_;

    mutable int _cached_size_;
    ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

    friend void  protobuf_AddDesc_message_2eproto();
    friend void protobuf_AssignDesc_message_2eproto();
    friend void protobuf_ShutdownFile_message_2eproto();

    void InitAsDefaultInstance();
    static Response *default_instance_;
};
// ===================================================================


// ===================================================================

// Request_Field

// required string name = 1;
inline bool Request_Field::has_name() const
{
    return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Request_Field::set_has_name()
{
    _has_bits_[0] |= 0x00000001u;
}
inline void Request_Field::clear_has_name()
{
    _has_bits_[0] &= ~0x00000001u;
}
inline void Request_Field::clear_name()
{
    if (name_ != &::google::protobuf::internal::kEmptyString)
    {
        name_->clear();
    }
    clear_has_name();
}
inline const ::std::string &Request_Field::name() const
{
    return *name_;
}
inline void Request_Field::set_name(const ::std::string &value)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(value);
}
inline void Request_Field::set_name(const char *value)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(value);
}
inline void Request_Field::set_name(const char *value, size_t size)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Request_Field::mutable_name()
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    return name_;
}
inline ::std::string *Request_Field::release_name()
{
    clear_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = name_;
        name_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// required string value = 2;
inline bool Request_Field::has_value() const
{
    return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Request_Field::set_has_value()
{
    _has_bits_[0] |= 0x00000002u;
}
inline void Request_Field::clear_has_value()
{
    _has_bits_[0] &= ~0x00000002u;
}
inline void Request_Field::clear_value()
{
    if (value_ != &::google::protobuf::internal::kEmptyString)
    {
        value_->clear();
    }
    clear_has_value();
}
inline const ::std::string &Request_Field::value() const
{
    return *value_;
}
inline void Request_Field::set_value(const ::std::string &value)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(value);
}
inline void Request_Field::set_value(const char *value)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(value);
}
inline void Request_Field::set_value(const char *value, size_t size)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Request_Field::mutable_value()
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    return value_;
}
inline ::std::string *Request_Field::release_value()
{
    clear_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = value_;
        value_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// -------------------------------------------------------------------

// Request

// required string tablename = 1;
inline bool Request::has_tablename() const
{
    return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Request::set_has_tablename()
{
    _has_bits_[0] |= 0x00000001u;
}
inline void Request::clear_has_tablename()
{
    _has_bits_[0] &= ~0x00000001u;
}
inline void Request::clear_tablename()
{
    if (tablename_ != &::google::protobuf::internal::kEmptyString)
    {
        tablename_->clear();
    }
    clear_has_tablename();
}
inline const ::std::string &Request::tablename() const
{
    return *tablename_;
}
inline void Request::set_tablename(const ::std::string &value)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(value);
}
inline void Request::set_tablename(const char *value)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(value);
}
inline void Request::set_tablename(const char *value, size_t size)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Request::mutable_tablename()
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    return tablename_;
}
inline ::std::string *Request::release_tablename()
{
    clear_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = tablename_;
        tablename_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// repeated .Request.Field fields = 2;
inline int Request::fields_size() const
{
    return fields_.size();
}
inline void Request::clear_fields()
{
    fields_.Clear();
}
inline const ::Request_Field &Request::fields(int index) const
{
    return fields_.Get(index);
}
inline ::Request_Field *Request::mutable_fields(int index)
{
    return fields_.Mutable(index);
}
inline ::Request_Field *Request::add_fields()
{
    return fields_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::Request_Field > &
Request::fields() const
{
    return fields_;
}
inline ::google::protobuf::RepeatedPtrField< ::Request_Field > *
Request::mutable_fields()
{
    return &fields_;
}

// -------------------------------------------------------------------

// Response_Field

// required string name = 1;
inline bool Response_Field::has_name() const
{
    return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Response_Field::set_has_name()
{
    _has_bits_[0] |= 0x00000001u;
}
inline void Response_Field::clear_has_name()
{
    _has_bits_[0] &= ~0x00000001u;
}
inline void Response_Field::clear_name()
{
    if (name_ != &::google::protobuf::internal::kEmptyString)
    {
        name_->clear();
    }
    clear_has_name();
}
inline const ::std::string &Response_Field::name() const
{
    return *name_;
}
inline void Response_Field::set_name(const ::std::string &value)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(value);
}
inline void Response_Field::set_name(const char *value)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(value);
}
inline void Response_Field::set_name(const char *value, size_t size)
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    name_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Response_Field::mutable_name()
{
    set_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        name_ = new ::std::string;
    }
    return name_;
}
inline ::std::string *Response_Field::release_name()
{
    clear_has_name();
    if (name_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = name_;
        name_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// required string value = 2;
inline bool Response_Field::has_value() const
{
    return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Response_Field::set_has_value()
{
    _has_bits_[0] |= 0x00000002u;
}
inline void Response_Field::clear_has_value()
{
    _has_bits_[0] &= ~0x00000002u;
}
inline void Response_Field::clear_value()
{
    if (value_ != &::google::protobuf::internal::kEmptyString)
    {
        value_->clear();
    }
    clear_has_value();
}
inline const ::std::string &Response_Field::value() const
{
    return *value_;
}
inline void Response_Field::set_value(const ::std::string &value)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(value);
}
inline void Response_Field::set_value(const char *value)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(value);
}
inline void Response_Field::set_value(const char *value, size_t size)
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    value_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Response_Field::mutable_value()
{
    set_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        value_ = new ::std::string;
    }
    return value_;
}
inline ::std::string *Response_Field::release_value()
{
    clear_has_value();
    if (value_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = value_;
        value_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// -------------------------------------------------------------------

// Response_Record

// repeated .Response.Field fields = 1;
inline int Response_Record::fields_size() const
{
    return fields_.size();
}
inline void Response_Record::clear_fields()
{
    fields_.Clear();
}
inline const ::Response_Field &Response_Record::fields(int index) const
{
    return fields_.Get(index);
}
inline ::Response_Field *Response_Record::mutable_fields(int index)
{
    return fields_.Mutable(index);
}
inline ::Response_Field *Response_Record::add_fields()
{
    return fields_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::Response_Field > &
Response_Record::fields() const
{
    return fields_;
}
inline ::google::protobuf::RepeatedPtrField< ::Response_Field > *
Response_Record::mutable_fields()
{
    return &fields_;
}

// -------------------------------------------------------------------

// Response

// required string tablename = 1;
inline bool Response::has_tablename() const
{
    return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Response::set_has_tablename()
{
    _has_bits_[0] |= 0x00000001u;
}
inline void Response::clear_has_tablename()
{
    _has_bits_[0] &= ~0x00000001u;
}
inline void Response::clear_tablename()
{
    if (tablename_ != &::google::protobuf::internal::kEmptyString)
    {
        tablename_->clear();
    }
    clear_has_tablename();
}
inline const ::std::string &Response::tablename() const
{
    return *tablename_;
}
inline void Response::set_tablename(const ::std::string &value)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(value);
}
inline void Response::set_tablename(const char *value)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(value);
}
inline void Response::set_tablename(const char *value, size_t size)
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    tablename_->assign(reinterpret_cast<const char *>(value), size);
}
inline ::std::string *Response::mutable_tablename()
{
    set_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        tablename_ = new ::std::string;
    }
    return tablename_;
}
inline ::std::string *Response::release_tablename()
{
    clear_has_tablename();
    if (tablename_ == &::google::protobuf::internal::kEmptyString)
    {
        return NULL;
    }
    else
    {
        ::std::string *temp = tablename_;
        tablename_ = const_cast< ::std::string *>(&::google::protobuf::internal::kEmptyString);
        return temp;
    }
}

// repeated .Response.Record records = 2;
inline int Response::records_size() const
{
    return records_.size();
}
inline void Response::clear_records()
{
    records_.Clear();
}
inline const ::Response_Record &Response::records(int index) const
{
    return records_.Get(index);
}
inline ::Response_Record *Response::mutable_records(int index)
{
    return records_.Mutable(index);
}
inline ::Response_Record *Response::add_records()
{
    return records_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::Response_Record > &
Response::records() const
{
    return records_;
}
inline ::google::protobuf::RepeatedPtrField< ::Response_Record > *
Response::mutable_records()
{
    return &records_;
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google
{
namespace protobuf
{


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_message_2eproto__INCLUDED