#ifndef JSE_NODE_H
#define JSE_NODE_H

namespace js {

	class Node
	{
	public:
		enum class Type {
			EMPTY, CAMERA, LIGHT, MESH, SKIN, WEIGHTS
		};
	private:
		JsString name_;
		Node* pParent_;
		JsVector<int> children_;
		vec3 translation_{ 0.0f };
		vec3 scale_{ 1.0f };
		quat rotation_{};
		vec3 saved_translation_{};
		vec3 saved_scale_{};
		quat saved_rotation_{};
		mat4 transformMatrixCache_{ 1.0f };

		Type type_{ Type::EMPTY };
		int index_{ -1 };

		bool dirty_{};
		bool animated_{};
		void make_dirty();
	public:
		Node();
		Node(const Node& other) = default;
		Node(Node&& other) = default;
		Node& operator=(const Node& other) = default;
		Node& operator=(Node&& other) = default;

		Node(Type type, const JsString& name, Node* pParent);
		JsString name() const;
		Node* parentNode();
		void SetMatrix(const mat4& m);
		void SetParent(Node* n);
		void SetIndex(int x);
		void SetType(Type t);
		const JsVector<int>& children() const;
		void AddChild(int x);
		vec3 translation() const;
		vec3 scale() const;
		quat rotation() const;
		bool animated() const;
		void BeginAnimate();
		void EndAnimate();
		void Scale(const vec3& v);
		void Translate(const vec3& v);
		void Rotate(const quat& v);
		void RotateX(float rad);
		void RotateY(float rad);
		void RotateZ(float rad);
		mat4 GetTransform();
		int index() const;
		Type type() const;
		bool isCamera() const;
		bool isLight() const;
		bool isMesh() const;
		bool empty() const;

		int operator[](size_t x) const;
	};
}
#endif // !JSE_NODE_H
