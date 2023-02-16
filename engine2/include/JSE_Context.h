#ifndef JSE_CONTEXT_H
#define JSE_CONTEXT_H

class JseContext {
public:
	JseContext() = default;
	~JseContext();
	JseModule* addModule(JseUniquePtr<JseModule> m);

	template <typename T>
	T* module() {
		auto it = map_.find(std::type_index(typeid(T)));
		if (it == map_.end()) {
			throw std::runtime_error("FATAL: Modul not found !!!");
		}

		return static_cast<T*>(it->second.get());
	}

	template <typename T, typename... Args>
	T* registerModule(Args... args) {
		return static_cast<T*>(addModule(std::make_unique<T>(std::forward<Args>(args)...)));
	}

private:
	JseHashMap<JseType, JseUniquePtr<JseModule>> map_;
};

extern JseContext appCtx;
#endif