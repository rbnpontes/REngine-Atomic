#pragma once
#include <EngineCore/Container/TypeTraits.h>

namespace REngine
{
	class JsonPrimitive;
	class JsonArray;

	enum class JsonObjectType
	{
		Object = 0,
		Array,
		Primitive
	};

	class IJsonBuilderObject
	{
	public:
		virtual IJsonBuilderObject* GetParent() const = 0;
		virtual JsonObjectType GetType() = 0;
		virtual ea::string ToString() const = 0;
	};

	class JsonObject : public IJsonBuilderObject
	{
	public:
		JsonObject() : parent_(nullptr) {}
		JsonObject(IJsonBuilderObject* parent) : parent_(parent){}
		~JsonObject() = default;
		ea::shared_ptr<JsonPrimitive> AddPrimitive(const ea::string& prop_name);
		ea::shared_ptr<JsonObject> AddObject(const ea::string& prop_name);
		ea::shared_ptr<JsonArray> AddArray(const ea::string& prop_name);
		// IJsonBuilderObject
		IJsonBuilderObject* GetParent() const override { return parent_; }
		JsonObjectType GetType() override { return JsonObjectType::Object; }
		ea::string ToString() const override;
	private:
		ea::unordered_map<ea::string, ea::shared_ptr<IJsonBuilderObject>> properties_;

		IJsonBuilderObject* parent_;
	};

	class JsonArray : public IJsonBuilderObject
	{
	public:
		JsonArray() : parent_(nullptr) {}
		JsonArray(IJsonBuilderObject* parent) : parent_(parent){}
		~JsonArray() = default;
		ea::shared_ptr<JsonPrimitive> PushPrimitive();
		ea::shared_ptr<JsonObject> PushObject();
		ea::shared_ptr<JsonArray> PushArray();
		// IJsonBuilderObject
		IJsonBuilderObject* GetParent() const override { return parent_; }
		JsonObjectType GetType() override { return JsonObjectType::Object; }
		ea::string ToString() const override;
	private:
		ea::vector<ea::shared_ptr<IJsonBuilderObject>> values_;

		IJsonBuilderObject* parent_;
	};

	class JsonPrimitive : public IJsonBuilderObject
	{
	public:
		JsonPrimitive() = default;
		void SetValue(const ea::string& value);
		void SetValue(double value);
		void SetValue(bool value);
		// IJsonBuilderObject
		IJsonBuilderObject* GetParent() const override { return nullptr; }
		JsonObjectType GetType() override { return JsonObjectType::Primitive; }
		ea::string ToString() const override;
	private:
		ea::string value_;
	};

	class JsonBuilder
	{
	public:
		static ea::shared_ptr<JsonObject> GetObject();
		static ea::shared_ptr<JsonArray> GetArray();
	};
}
