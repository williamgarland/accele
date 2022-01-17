#pragma once

namespace acl {
template <typename T>
struct StrongRef {
	T* obj;
	StrongRef(T* obj) : obj(obj) { ++(obj->rcount); }
	~StrongRef() {
		if (obj) {
			--(obj->rcount);
			if (obj->rcount == 0) delete obj;
		}
	}
	T* release() {
		auto tmp = obj;
		obj = nullptr;
		return tmp;
	}
	T* operator->() { return obj; }
	T& operator*() { return *obj; }
};

class Any;
int getRefTableCount(Any* obj);

template <typename T>
struct WeakRef {
	T* obj;
	WeakRef(const StrongRef<T>& ref) : obj(ref.obj) {
		obj->count.useTable(static_cast<Any*>(obj));
	}
	bool has() const { return getRefTableCount(static_cast<Any*>(obj)) > 0; }
	StrongRef<T> acquire() {
		if (getRefTableCount(static_cast<Any*>(obj)) == 0)
			throw "Nil reference";
		return StrongRef<T>(obj);
	}
};

template <typename T>
struct GreedyRef {
	T* obj;
	GreedyRef(T* obj) : obj(obj) {}
	~GreedyRef() {
		if (obj) {
			delete obj;
		}
	}
	T* release() {
		auto tmp = obj;
		obj = nullptr;
		return tmp;
	}
	T* operator->() { return obj; }
	T& operator*() { return *obj; }
};

template <typename T>
struct ObserverRef {
	T* obj;
	ObserverRef(T* obj) : obj(obj) {}
	ObserverRef(const StrongRef<T>& ref) : obj(ref.obj) {}
	ObserverRef(const GreedyRef<T>& ref) : obj(ref.obj) {}
	ObserverRef(const WeakRef<T>& ref) : obj(ref.acquire().obj) {}
	T* operator->() { return obj; }
	T& operator*() { return *obj; }
};
}  // namespace acl