#ifndef JSE_NODE_H
#define JSE_NODE_H

enum class JseNodeType {
	EMPTY,CAMERA,LIGHT,MESH,SKIN,WEIGHTS
};

class JseNode
{
private:
	JseString name_;
	JseNode* pParent_;
	JseVector<int> children_;
	vec3 translation_{ 0.0f };
	vec3 scale_{ 1.0f };
	quat rotation_{};
	vec3 saved_translation_{};
	vec3 saved_scale_{};
	quat saved_rotation_{};
	mat4 transformMatrixCache_{ 1.0f };

	JseNodeType type_{JseNodeType::EMPTY};
	int index_{ -1 };

	bool dirty_{};
	bool animated_{};
	void make_dirty();
public:
	JseNode();
	JseNode(const JseNode& other) = default;
	JseNode(JseNode&& other) = default;
	JseNode& operator=(const JseNode& other) = default;
	JseNode& operator=(JseNode&& other) = default;

	JseNode(JseNodeType type, const JseString& name, JseNode* pParent);
	JseString name() const;
	JseNode* parentNode();
	void SetMatrix(const mat4& m);
	void SetParent(JseNode* n);
	void SetIndex(int x);
	void SetType(JseNodeType t);
	const JseVector<int>& children() const;
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
	JseNodeType type() const;
	bool isCamera() const;
	bool isLight() const;
	bool isMesh() const;
	bool empty() const;

	int operator[](size_t x) const;
};
#endif // !JSE_NODE_H
