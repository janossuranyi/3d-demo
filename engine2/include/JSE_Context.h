#ifndef JSE_CONTEXT_H
#define JSE_CONTEXT_H

namespace js {
	class Context {
	public:
		Context() = default;
		~Context();
		Module* AddModule(JsUniquePtr<Module> m);

		template <typename T>
		T* module() {
			auto it = map_.find(std::type_index(typeid(T)));
			if (it == map_.end()) {
				throw std::runtime_error("FATAL: Modul not found !!!");
			}

			return static_cast<T*>(it->second.get());
		}

		template <typename T, typename... Args>
		T* RegisterModule(Args... args) {
			return static_cast<T*>(AddModule(std::make_unique<T>(std::forward<Args>(args)...)));
		}

	private:
		JsHashMap<JsType, JsUniquePtr<Module>> map_;
	};
}

extern js::Context appCtx;
#endif
