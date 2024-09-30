using System.Text;
using BindingGenerator.Models;

namespace BindingGenerator.Utils;

public static class BindingFileUtils
{
    public static readonly string GeneratedFileComment = "Don't touch on this file, the file is generated automatically\nby the binding generator.";
    public static string GetFileName(NamespaceDefinition? typeNamespace, TypeDefinition type)
    {
        var nsChain = CodeUtils.GetNamespaceChain(typeNamespace);
        if (!string.IsNullOrEmpty(nsChain))
            nsChain += '_';
        nsChain += Enum.GetName(type.Kind) ?? string.Empty;
        nsChain += "_";
        return nsChain + type.Name;
    }

}