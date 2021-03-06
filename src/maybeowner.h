#pragma once

#include <memory>
#include <cassert>

namespace omm
{

template<typename T>
class MaybeOwner
{
public:
  MaybeOwner(const MaybeOwner<T>& other) = delete;
  MaybeOwner(MaybeOwner<T>&& other) = default;
  MaybeOwner(std::unique_ptr<T> own) : m_owned(std::move(own)), m_ref(*m_owned) { }
  MaybeOwner(T& reference) : m_ref(reference) { }
  operator T&() const { return m_ref; }
  T& get() const { return *this; }
  bool owns() const { return !!m_owned.get(); }
  auto release() { assert(owns()); return std::move(m_owned); }
  MaybeOwner& operator=(MaybeOwner&& other) = default;
  T* operator ->() { return &m_ref.get(); }

  T& capture_by_copy()
  {
    m_owned = m_ref.get().clone();
    m_ref = *m_owned;
    return m_ref;
  }

  T& capture(std::unique_ptr<T> own)
  {
    assert(!owns());
    assert(&get() == &*own);
    m_owned = std::move(own);
    assert(owns());
    return *this;
  }

private:
  std::unique_ptr<T> m_owned;
  std::reference_wrapper<T> m_ref;
};

}  // namespace omm
