using System.Text;
using BindingGenerator.Models;

namespace BindingGenerator.Utils;

public static class BindingFileUtils
{
    public static string GetFileName(NamespaceDefinition? typeNamespace, TypeDefinition type)
    {
        var nsChain = CodeUtils.GetNamespaceChain(typeNamespace);
        if (!string.IsNullOrEmpty(nsChain))
            nsChain += '_';
        return nsChain + type.Name;
    }

}