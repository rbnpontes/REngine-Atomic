using System.Text;
using BindingGenerator.CodeBuilders;
using BindingGenerator.Models;

namespace BindingGenerator.Utils;

public static class CodeUtils
{
    private static string BuildNamespaceChain(NamespaceDefinition? ns, Func<string, string> transform)
    {
        var result = string.Empty;
        while (!string.IsNullOrEmpty(ns?.Name))
        {
            if (!string.IsNullOrEmpty(result))
                result = "_" + result;
            result = transform(ns?.Name ?? string.Empty) + result;
            ns = ns?.Owner;
        }

        return result;
    }
    
    public static string GetNamespaceChain(NamespaceDefinition? ns)
    {
        return BuildNamespaceChain(ns, (x) => x);
    }

    public static string GetSnakeCaseNamespaceChain(NamespaceDefinition? ns)
    {
        return BuildNamespaceChain(ns, (x) => CodeUtils.ToSnakeCase(x));
    }

    public static string GetMethodDeclName(NamespaceDefinition? ns, TypeDefinition typeDef)
    {
        var nsChain = GetSnakeCaseNamespaceChain(ns);
        if (!string.IsNullOrEmpty(nsChain))
            nsChain += '_';
        return nsChain + ToSnakeCase(typeDef.Name);
    }
    
    public static string ToCamelCase(string input)
    {
        if (input.Length == 0)
            return string.Empty;
        char[] chars = input.ToCharArray();
        chars[0] = char.ToLower(chars[0]);
        return new string(chars);
    }
    public static string ToSnakeCase(string input)
    {
        var output = new StringBuilder();

        var isUpper = false;
        for(var i = 0; i < input.Length; ++i)
        {
            if ((char.IsUpper(input[i]) ||char.IsNumber(input[i])) && !isUpper)
            {
                if(i > 0)
                    output.Append('_');
                isUpper = true;
            }
            else if (char.IsLower(input[i]))
            {
                isUpper = false;
            }

            output.Append(char.ToLowerInvariant(input[i]));
        }

        return output.ToString();
    }

    public static void WriteCode(string path, string code)
    {
        var dir = Path.GetDirectoryName(path);
        if (!Directory.Exists(dir) && dir is not null)
            Directory.CreateDirectory(dir);
        if(File.Exists(path))
            File.Delete(path);
        
        
        File.WriteAllText(path, code);
    }

    public static void WriteCode(string path, CppBuilder builder)
    {
        WriteCode(path, builder.ToString());
    }
}