/*
 * frame_event_packet_iterator.inl
 *
 *  Created on: Mar 7, 2017
 *      Author: S. Isler (stefan@insightness.com), Insightness AG
 */

#ifndef TYPES_SER_EVENTS_IMPL_FRAME_EVENT_PACKET_ITERATOR_INL_
#define TYPES_SER_EVENTS_IMPL_FRAME_EVENT_PACKET_ITERATOR_INL_

namespace iness
{


/*! Basic bidirectional iterator functionality for frame event packets that is shared by both const and non-const iterators.
 *
 */
template<class CRTP_CHILD>
class FrameEventPacket::IteratorBase: public std::iterator<std::bidirectional_iterator_tag, FrameEvent>
{
public:
  //! Empty constructor
  IteratorBase()
  : packet_(nullptr)
  , pos_(0)
  {}


  /*! Constructor
   */
  IteratorBase(FrameEventPacket* _pkt, unsigned int _pos = 0)
  : packet_(_pkt)
  , pos_(_pos)
  {}

  /*! Advances the iterator by one.
   */
  CRTP_CHILD& operator++()
  {
    ++pos_;
    return static_cast<CRTP_CHILD&>(*this);
  }

  /*! Advances the iterator by one.
   */
  CRTP_CHILD operator++(int)
  {
    CRTP_CHILD old = static_cast<CRTP_CHILD>(*this);
    ++pos_;
    return old;
  }

  /*! Decrements iterator.
   */
  CRTP_CHILD& operator--()
  {
    --pos_;
    return static_cast<CRTP_CHILD&>(*this);
  }

  //! Decrements iterator.
  CRTP_CHILD operator--(int)
  {
    CRTP_CHILD old = static_cast<CRTP_CHILD>(*this);
    --pos_;
    return old;
  }

  //! Iterator comparison.
  bool operator==(CRTP_CHILD _it) const
  {
    return _it.packet_==packet_ && _it.pos_==pos_;
  }

  //! Iterator comparison.
  bool operator!=(CRTP_CHILD _it) const
  {
    return _it.packet_!=packet_ || _it.pos_!=pos_;
  }

protected:
  mutable FrameEventPacket* packet_; //!< Points to the frame event packet over which the iterator iterates.
  unsigned int pos_; //! Position within the packet to which the iterator points.
};

/*! Const iterator to access elements within a const FrameEventPacket with read-only access.
 *
 */
class FrameEventPacket::ConstIterator: public FrameEventPacket::template IteratorBase<ConstIterator>
{
public:
  //! Empty constructor
  ConstIterator()=default;

  /*! Constructor
   */
  ConstIterator(const FrameEventPacket* _pkt, unsigned int _pos): IteratorBase(const_cast<FrameEventPacket*>(_pkt),_pos){}

  //! Element access.
  const FrameEvent& operator*() const
  {
    return (*packet_)[this->pos_];
  }

  //! Element access.
  const FrameEvent* operator->() const
  {
    return &operator*();
  }
};

/*! Iterator to access elements within a non-const FrameEventPacket with read- and write-access.
 *
 */
class FrameEventPacket::Iterator: public FrameEventPacket::template IteratorBase<Iterator>
{
public:
  //! Empty constructor
  Iterator()=default;

  /*! Constructor
   */
  Iterator(FrameEventPacket* _pkt, unsigned int _pos): IteratorBase(_pkt,_pos){}

  //! Conversion to ConstIterator
  operator ConstIterator() const
  {
    return ConstIterator(this->packet_,this->pos_);
  }

  //! Element access.
  FrameEvent& operator*()
  {
    return (*packet_)[this->pos_];
  }

  //! Element access.
  FrameEvent* operator->()
  {
    return &operator*();
  }
};

}

#endif /* TYPES_SER_EVENTS_IMPL_FRAME_EVENT_PACKET_ITERATOR_INL_ */
