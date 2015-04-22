using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using YamlDotNet.Core;
using YamlDotNet.RepresentationModel;

namespace modelconverter
{
    static class SceneImporter
    {
        public static void Import(string filePath)
        {
            var clean_file = new StringWriter();
            Console.WriteLine("Cleaning YAML input...");
            using (var file = new StreamReader(filePath))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                {
                    if (line.StartsWith("---"))
                    { // document start?
                        Regex myRegex = new Regex(@"(!u![0-9]*)", RegexOptions.None);
                        clean_file.WriteLine(myRegex.Replace(line, ""));
                    }
                    else
                    {
                        clean_file.WriteLine(line);
                    }
                }
            }
            Console.WriteLine("Parsing file...");
            var yaml = new YamlStream();
            yaml.Load(new StringReader(clean_file.ToString()));

            foreach (var doc in yaml.Documents)
            {
                Console.WriteLine(doc.RootNode.Anchor);
                
            }
        }
    }
}
