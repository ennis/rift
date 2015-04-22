using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Assimp;
using System.IO;
using System.Xml.Serialization;
using System.Diagnostics;
using System.Windows.Forms;

namespace modelconverter
{
    class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            var options = new ProgramOptions();
            if (!CommandLine.Parser.Default.ParseArguments(args, options))
            {
                //Console.WriteLine("Error reading arguments");
                return;
            }
            if (options.InputFiles.Count == 0)
            {
                var ofd = new OpenFileDialog();
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    options.InputFiles.Add(ofd.FileName);
                }
            }
            if (options.InputFiles.Count == 0)
                return;
            // are we loading a unity scene?
            if (Path.GetExtension(options.InputFiles[0]) == ".unity")
            {
                SceneImporter.Import(options.InputFiles[0]);
            }
            else
            {
                options.OutputDirectory = options.OutputDirectory ?? Path.GetDirectoryName(options.InputFiles[0]);
                options.ModelName = options.ModelName ?? Path.GetFileNameWithoutExtension(options.InputFiles[0]);
                AssimpContext importer = new AssimpContext();
                LogStream logstream = new LogStream(delegate(String msg, String userData)
                {
                    Debug.WriteLine(msg);
                });
                logstream.Attach();
                if (options.Verbose)
                {
                    Console.WriteLine("*** Master model file: " + options.InputFiles[0]);
                    Console.WriteLine("*** Additional animation files: ");
                    foreach (var file in options.InputFiles.Skip(1))
                    {
                        Console.WriteLine(file);
                    }
                    Console.WriteLine("*** Model name: " + options.ModelName);
                    Console.WriteLine("*** Output directory: " + options.OutputDirectory);
                }

                // the conversion is done in two steps:
                // first, we convert the master model file and extract its skeleton
                // We assign an ID to each bone: these will correspond to the track ID in the exported animations
                // We also export animations contained in the primary file
                // Then, we export all animations contained in the additional files
                // These animations must match the skeleton of the master model file

                // Import master model 
                Scene masterScene = importer.ImportFile(options.InputFiles[0],
                    PostProcessSteps.OptimizeGraph |
                    PostProcessSteps.SortByPrimitiveType |
                    PostProcessSteps.Triangulate |
                    PostProcessSteps.OptimizeMeshes |
                    PostProcessSteps.CalculateTangentSpace);

                ModelExporter.Export(masterScene, options);
                AnimationExporter.Export(masterScene, options);

                foreach (var file in options.InputFiles.Skip(1))
                {
                    Scene scene = importer.ImportFile(file, PostProcessSteps.OptimizeGraph | PostProcessSteps.OptimizeMeshes);
                    AnimationExporter.Export(scene, options);
                }

                Console.WriteLine("*** Press any key ***");
                Console.ReadKey();
            }
        }
    }
}
