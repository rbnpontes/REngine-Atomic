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
}