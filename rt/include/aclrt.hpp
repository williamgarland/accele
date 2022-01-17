#pragma once

#include "arithops.hpp"
#include "objbase.hpp"
#include "ptypes.hpp"

namespace acl {
class Type : public Any {
   public:
	virtual ~Type();
};

class Number : public Any {
   public:
	virtual ~Number();
};

struct Int : public Number {
	pint value;
	Int(pint value);
	virtual ~Int();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(Int);
};

struct Int8 : public Number {
	pint8 value;
	Int8(pint8 value);
	virtual ~Int8();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(Int8);
};

struct Int16 : public Number {
	pint16 value;
	Int16(pint16 value);
	virtual ~Int16();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(Int16);
};

struct Int32 : public Number {
	pint32 value;
	Int32(pint32 value);
	virtual ~Int32();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(Int32);
};

struct Int64 : public Number {
	pint64 value;
	Int64(pint64 value);
	virtual ~Int64();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(Int64);
};

struct UInt : public Number {
	puint value;
	UInt(puint value);
	virtual ~UInt();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(UInt);
};

struct UInt8 : public Number {
	puint8 value;
	UInt8(puint8 value);
	virtual ~UInt8();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(UInt8);
};

struct UInt16 : public Number {
	puint16 value;
	UInt16(puint16 value);
	virtual ~UInt16();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(UInt16);
};

struct UInt32 : public Number {
	puint32 value;
	UInt32(puint32 value);
	virtual ~UInt32();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(UInt32);
};

struct UInt64 : public Number {
	puint64 value;
	UInt64(puint64 value);
	virtual ~UInt64();
	virtual StrongRef<String> asString() const override;
	ACL_RT_INTOPS_DECL(UInt64);
};

struct Float : public Number {
	pfloat value;
	Float(pfloat value);
	virtual ~Float();
	virtual StrongRef<String> asString() const override;
	ACL_RT_ARITHOPS_DECL(Float);
};

struct Double : public Number {
	pdouble value;
	Double(pdouble value);
	virtual ~Double();
	virtual StrongRef<String> asString() const override;
	ACL_RT_ARITHOPS_DECL(Double);
};

struct Float80 : public Number {
	pfloat80 value;
	Float80(pfloat80 value);
	virtual ~Float80();
	virtual StrongRef<String> asString() const override;
	ACL_RT_ARITHOPS_DECL(Float80);
};

struct Bool : public Any {
	pbool value;
	Bool(pbool value);
	virtual ~Bool();
	virtual StrongRef<String> asString() const override;
};

struct String : public Any {
	pstring value;
	String(const pstring& value);
	virtual ~String();
	virtual StrongRef<String> asString() const override;
	StrongRef<String> operator+(ObserverRef<String> other) const;
};

template <typename BoxedType, typename PType>
StrongRef<BoxedType> box(PType value) {
	return StrongRef<BoxedType>(new BoxedType(value));
}
}  // namespace acl

#ifndef ACL_RT_NO_UNDEF_ARITHOPS
#include "arithops_undef.hpp"
#endif