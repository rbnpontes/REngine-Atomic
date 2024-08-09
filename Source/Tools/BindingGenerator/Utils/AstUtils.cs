using BindingGenerator.Models;
using CppAst;

namespace BindingGenerator.Utils;

public static class AstUtils
{
    private static readonly HashSet<string> pOperators = new(
        [
            "+", "-","*","/","%","++","--","==","!=",">","<",">=","<=","<=>",
            "~","&","|","^","<<",">>","!","&&","||","=","+=","-=","*=",
            "/=","%=","&=","|=","^=","<<=",">>=","[]","->","->*","new","new[]",
            "delete", "delete[]"
        ]
    );

    private static readonly Dictionary<string, VectorType> pVecTypes = new ()
    {
        { "vector", VectorType.Default },
        { "podvector", VectorType.Pod } 
    };
    public static bool IsOperatorMethod(CppFunction function)
    {
        var expectedOp = function.Name.Replace("operator", string.Empty);
        return pOperators.Contains(expectedOp);
    }

    public static bool IsIgnoredType(ICppAttributeContainer attributeContainer)
    {
        var arg = attributeContainer.MetaAttributes.QueryArgument("engine_bind_ignore");
        return arg != null;
    }

    public static bool IsStringHash(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "stringhash");
    }

    public static bool IsString(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "string");
    }

    public static bool IsVector(CppType type)
    {
        return GetVectorType(type) != VectorType.Undefined;
    }

    public static VectorType GetVectorType(CppType type)
    {
        if (pVecTypes.TryGetValue(type.GetDisplayName().ToLowerInvariant(), out var vecType))
            return vecType;
        return VectorType.Undefined;
    }
}