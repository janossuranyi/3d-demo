#ifndef JSE_CONTEXT_H
#define JSE_CONTEXT_H

class JseContext {
public:
	JseContext() = default;
	~JseContext();

private:
	std::unordered_map<Type, std::unique_ptr<JseModule>> map_;
};
#endif