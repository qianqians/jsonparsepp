/*
 * allocator .h
 *
 *  Created on: 2019-9-10
 *      Author: qianqians
 */
#ifndef _allocator_h
#define _allocator_h

#include <stdlib.h>
#include <memory.h>

namespace Fossilizid{
namespace JsonParse{

struct chunk {
	void * mem;
	size_t size;
	size_t offset;
	size_t count;
};
inline chunk * create_chunk(size_t size) {
	void * mem = malloc(size);
	memset(mem, 0, size);
	chunk * _c = (chunk *)mem;
	_c->mem = mem;
	_c->size = size;
	_c->offset = sizeof(chunk);
	_c->count = 0;

	return _c;
}
inline void * chunk_malloc(chunk * _c, size_t _size) {
	void ** mem = (void**)(&((char*)_c->mem)[_c->offset]);
	_c->offset += _size;
	++_c->count;
	*mem = _c;
	++mem;
	return mem;
}

template<class _Ty>
class allocator
{	// generic allocator for objects of class _Ty
public:
	using value_type = _Ty;

	typedef _Ty * pointer;
	typedef const _Ty * const_pointer;

	typedef _Ty& reference;
	typedef const _Ty& const_reference;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	template<class _Other>
	struct rebind
	{
		using other = allocator<_Other>;
	};

	constexpr allocator() noexcept
	{
		_c = create_chunk(4096);
	}

	constexpr allocator(const allocator& _o) noexcept
	{
		_c = _o._c;
	}

	template<class _Other>
	constexpr allocator(const allocator<_Other>& _Other) noexcept
	{
		_c = _Other._c;
	}

	void deallocate(_Ty * const _Ptr, const size_t _Count)
	{
		void ** _p = (void**)_Ptr;
		--_p;
		chunk * _c_ = *(chunk**)_p;
		--_c_->count;
		if (_c_->count <= 0) {
			free(_c_->mem);
		}
	}

	_Ty * allocate(const size_t _Count)
	{
		void * mem = nullptr;

		size_t _s = sizeof(_Ty)*_Count + sizeof(void*);
		if (_s > (_c->size-_c->offset))
		{
			size_t _stmp = (_s + 4095) / 4096 * 4096;
			chunk * _ctmp = create_chunk(_stmp);
			mem = chunk_malloc(_ctmp, _s);
			_c = (_c->size - _c->offset) > (_ctmp->size - _ctmp->offset) ? _c : _ctmp;
		}
		else {
			mem = chunk_malloc(_c, _s);
		}

		return (_Ty*)mem;
	}

	template<class _Objty, class... _Types>
	void construct(_Objty * const _Ptr, _Types&&... _Args)
	{
		::new (const_cast<void *>(static_cast<const volatile void *>(_Ptr))) _Objty(std::forward<_Types>(_Args)...);
	}

	template<class _Uty>
	void destroy(_Uty * const _Ptr)
	{
		_Ptr->~_Uty();
	}

public:
	chunk * _c;

};



} /* namespace JsonParse */
} /* namespace Fossilizid */

#endif //_allocator_h
