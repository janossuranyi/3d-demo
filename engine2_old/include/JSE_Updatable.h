#ifndef JSE_UPDATABLE_H
#define JSE_UPDATABLE_H

class JseUpdatable
{
public:
	virtual ~JseUpdatable() {}

	void OnInputEvent(const JseEvent& e);
private:
	virtual void OnInputEvent_impl(const JseEvent& e) = 0;
};

JSE_INLINE void JseUpdatable::OnInputEvent(const JseEvent& e) {
	OnInputEvent_impl(e);
}

#endif // !JSE_UPDATABLE_H
