using System.Text;
using BindingGenerator.CodeBuilders;

namespace BindingGenerator.Utils;

public static class CodeUtils
{
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
        if(File.Exists(path))
            File.Delete(path);
			
        File.WriteAllText(path, code);
    }

    public static void WriteCode(string path, CppBuilder builder)
    {
        WriteCode(path, builder.ToString());
    }
}