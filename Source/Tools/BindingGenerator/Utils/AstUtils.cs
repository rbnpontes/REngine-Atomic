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
        var expectedOp = function.Name.Replace("operator", string.Empty).Trim();
        return pOperators.Contains(expectedOp);
    }

    public static bool IsIgnoredType(ICppAttributeContainer attributeContainer)
    {
        var arg = attributeContainer.MetaAttributes.QueryArgument("engine_bind_ignore");
        return arg != null;
    }

    public static bool CanBeConstructable(CppClass klass)
    {
        // if class has dllimport attribute and has constructor
        // then can be constructable
        var hasDllImport = klass.Attributes.Find(x => string.Equals(x.Name, "dllimport")) is not null;
        if (hasDllImport && klass.Constructors.Count > 0)
            return true;
        // but if class doesn't have constructor but has a inline constructor, then it can be constructable.
        var inlineCtors = klass.Constructors.Where(x => (x.Flags & CppFunctionFlags.Inline) != 0);
        return inlineCtors.Any();
    }
}