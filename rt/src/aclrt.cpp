#define ACL_RT_NO_UNDEF_ARITHOPS
#include "../include/aclrt.hpp"

#include <sstream>

namespace acl {
Type::~Type() {}

Number::~Number() {}

Int::Int(pint value) : value(value) {}
Int::~Int() {}
ACL_RT_INTOPS_DEF(Int);

StrongRef<String> Int::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Int8::Int8(pint8 value) : value(value) {}
Int8::~Int8() {}
ACL_RT_INTOPS_DEF(Int8);

StrongRef<String> Int8::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Int16::Int16(pint16 value) : value(value) {}
Int16::~Int16() {}
ACL_RT_INTOPS_DEF(Int16);

StrongRef<String> Int16::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Int32::Int32(pint32 value) : value(value) {}
Int32::~Int32() {}
ACL_RT_INTOPS_DEF(Int32);

StrongRef<String> Int32::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Int64::Int64(pint64 value) : value(value) {}
Int64::~Int64() {}
ACL_RT_INTOPS_DEF(Int64);

StrongRef<String> Int64::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

UInt::UInt(puint value) : value(value) {}
UInt::~UInt() {}
ACL_RT_INTOPS_DEF(UInt);

StrongRef<String> UInt::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

UInt8::UInt8(puint8 value) : value(value) {}
UInt8::~UInt8() {}
ACL_RT_INTOPS_DEF(UInt8);

StrongRef<String> UInt8::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

UInt16::UInt16(puint16 value) : value(value) {}
UInt16::~UInt16() {}
ACL_RT_INTOPS_DEF(UInt16);

StrongRef<String> UInt16::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

UInt32::UInt32(puint32 value) : value(value) {}
UInt32::~UInt32() {}
ACL_RT_INTOPS_DEF(UInt32);

StrongRef<String> UInt32::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

UInt64::UInt64(puint64 value) : value(value) {}
UInt64::~UInt64() {}
ACL_RT_INTOPS_DEF(UInt64);

StrongRef<String> UInt64::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Float::Float(pfloat value) : value(value) {}
Float::~Float() {}
ACL_RT_ARITHOPS_DEF(Float);

StrongRef<String> Float::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Double::Double(pdouble value) : value(value) {}
Double::~Double() {}
ACL_RT_ARITHOPS_DEF(Double);

StrongRef<String> Double::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Float80::Float80(pfloat80 value) : value(value) {}
Float80::~Float80() {}
ACL_RT_ARITHOPS_DEF(Float80);

StrongRef<String> Float80::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

Bool::Bool(pbool value) : value(value) {}
Bool::~Bool() {}

StrongRef<String> Bool::asString() const {
	std::stringstream ss;
	ss << value;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}

String::String(const pstring& value) : value(value) {}
String::~String() {}

StrongRef<String> String::asString() const {
	return StrongRef<String>(new String(value));
}

StrongRef<String> String::operator+(ObserverRef<String> other) const {
	std::stringstream ss;
	ss << value;
	ss << other->value;
	pstring newstr = ss.str();
	return StrongRef<String>(new String(newstr));
}

}  // namespace acl