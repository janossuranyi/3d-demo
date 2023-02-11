#ifndef JSE_NODE_H
#define JSE_NODE_H

enum class JseNodeType {
	EMPTY,CAMERA,LIGHT,MESH,SKIN,WEIGHTS
};

class JseNode
{
private:
	JseString name_;
	int parent_{ -1 };
	JseNode* parent_inst_;
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

	JseNode(JseNodeType type);
	JseNode(JseNodeType type, const JseString& name);
	JseNode(JseNodeType type, const JseString& name, int parent);
	JseString name() const;
	int parent() const;
	JseNode* parentNode();
	void setParent(int x, JseNode* n);
	void setIndex(int x);
	void setType(JseNodeType t);
	const JseVector<int>& children() const;
	void addChild(int x);
	vec3 translation() const;
	vec3 scale() const;
	quat rotation() const;
	bool animated() const;
	void beginAnimate();
	void endAnimate();
	void scale(const vec3& v);
	void translate(const vec3& v);
	void rotate(const quat& v);
	void rotateX(float rad);
	void rotateY(float rad);
	void rotateZ(float rad);
	mat4 getTransform();
	int index() const;
	JseNodeType type() const;
	bool isCamera() const;
	bool isLight() const;
	bool isMesh() const;
	bool empty() const;

	int operator[](size_t x) const;
};
#endif // !JSE_NODE_H
