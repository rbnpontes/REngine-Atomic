#include "JsonBuilder.h"
#include <EngineCore/Core/StringUtils.h>

namespace REngine
{
	static ea::string json_builder__gen_indent(u32 size)
	{
		ea::string res;
		for (u32 i = 0; i < size; ++i)
			res.append("\t");
		return res;
	}
	static u32 json_builder__get_indent(const IJsonBuilderObject* obj)
	{
		u32 result = 0;
		while(obj)
		{
			++result;
			obj = obj->GetParent();
		}
		return result;
	}

	ea::shared_ptr<JsonPrimitive> JsonObject::AddPrimitive(const ea::string& prop_name)
	{
		auto primitive = ea::make_shared<JsonPrimitive>();
		properties_[prop_name] = primitive;
		return primitive;
	}
	ea::shared_ptr<JsonObject> JsonObject::AddObject(const ea::string& prop_name)
	{
		auto obj = ea::shared_ptr<JsonObject>(new JsonObject(this));
		properties_[prop_name] = obj;
		return obj;
	}
	ea::shared_ptr<JsonArray> JsonObject::AddArray(const ea::string& prop_name)
	{
		auto arr = ea::shared_ptr<JsonArray>(new JsonArray(this));
		properties_[prop_name] = arr;
		return arr;
	}

	ea::string JsonObject::ToString() const
	{
		ea::string result = "{\n";
		const auto indent_str = json_builder__gen_indent(json_builder__get_indent(this));
		u32 i = 0;

		for(const auto& it : properties_)
		{
			result.append(indent_str);
			result.append(it.first);
			result.append(" : ");
			result.append(it.second->ToString());

			if(i < properties_.size() - 1)
				result.append(",");
			++i;
			result.append("\n");
		}

		result.append(json_builder__gen_indent(json_builder__get_indent(parent_)));
		result.append("}");
		return result;
	}

	ea::shared_ptr<JsonPrimitive> JsonArray::PushPrimitive()
	{
		auto primitive = ea::make_shared<JsonPrimitive>();
		values_.push_back(primitive);
		return primitive;
	}
	ea::shared_ptr<JsonObject> JsonArray::PushObject()
	{
		auto obj = ea::shared_ptr<JsonObject>(new JsonObject(this));
		values_.push_back(obj);
		return obj;
	}
	ea::shared_ptr<JsonArray> JsonArray::PushArray()
	{
		auto arr = ea::shared_ptr<JsonArray>(new JsonArray(this));
		values_.push_back(arr);
		return arr;
	}

	ea::string JsonArray::ToString() const
	{
		ea::string result = "[\n";
		const auto indent_str = json_builder__gen_indent(json_builder__get_indent(this));

		const auto values_size = values_.size();
		for(u32 i = 0; i < values_size; ++i)
		{
			result.append(indent_str);
			result.append(values_[i]->ToString());
			if (i < values_size - 1)
				result.append(",");
			result.append("\n");
		}

		result.append(json_builder__gen_indent(json_builder__get_indent(parent_)));
		result.append("]");

		return result;
	}

	void JsonPrimitive::SetValue(const ea::string& value)
	{
		value_ = value;
	}
	void JsonPrimitive::SetValue(bool value)
	{
		value_ = value ? "true" : "false";
	}
	void JsonPrimitive::SetValue(double value)
	{
		value_.clear();
		value_.append(Atomic::ToString("%f", value).CString());
	}

	ea::string JsonPrimitive::ToString() const
	{
		return value_;
	}

	ea::shared_ptr<JsonObject> JsonBuilder::GetObject()
	{
		return ea::make_shared<JsonObject>();
	}
	ea::shared_ptr<JsonArray> JsonBuilder::GetArray()
	{
		return ea::make_shared<JsonArray>();
	}
}
