using CommandLine;
using CommandLine.Text;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace modelconverter
{
    class ProgramOptions
    {
        [ValueList(typeof(List<string>))]
        public List<string> InputFiles { get; set; }

        [Option('s', "scene", Required=false, HelpText = "Import unity scene from input directory.")]
        public bool ImportUnityScene { get; set; }

        [Option('o', "output-dir", Required = false, HelpText = "Output directory.")]
        public string OutputDirectory { get; set; }

        [Option('m', "model-name", Required = false, HelpText = "Model name override.")]
        public string ModelName { get; set; }

        [Option('v', "verbose", Required = false, HelpText = "Verbose mode.", DefaultValue = false)]
        public bool Verbose { get; set; }
       
        [HelpOption]
        public string GetUsage()
        {
            var help = new HelpText
            {
                Heading = new HeadingInfo("<>", "<>"),
                Copyright = new CopyrightInfo("<>", 2012),
                AdditionalNewLineAfterOption = true,
                AddDashesToOption = true
            };
            help.AddPreOptionsLine("<>");
            help.AddPreOptionsLine("Usage: modelconverter <options> <input-files>");
            help.AddOptions(this);
            return help;
        }
    }
}
