using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml.Serialization;

namespace modelconverter
{
    static class AnimationExporter
    {
        public static void Export(Assimp.Scene scene, ProgramOptions options)
        {
            bool verb = options.Verbose;
            if (verb) Console.WriteLine("Exporting Animations...");
            if (!Directory.Exists(options.OutputDirectory))
                Directory.CreateDirectory(options.OutputDirectory);

            foreach (var anim in scene.Animations) 
            {
                var path = Path.Combine(options.OutputDirectory, options.ModelName + "@" + anim.Name + ".anim");
                Console.WriteLine("Writing " + path + "...");
                using (var outputFile = new FileStream(path, FileMode.Create, FileAccess.Write)) {
                    var writer = new BinaryWriter(outputFile);
                    // TODO overwrite animation name
                    writer.Write(anim.Name);
                    writer.Write(anim.NodeAnimationChannelCount);
                    foreach (var channel in anim.NodeAnimationChannels)
                    {
                        Console.WriteLine(anim.Name + " / " + channel.NodeName);
                        writer.Write(channel.NodeName);
                        writer.Write(channel.PositionKeyCount);
                        foreach (var key in channel.PositionKeys) 
                        { 
                            writer.Write((float)key.Time);
                            writer.Write(key.Value.X); 
                            writer.Write(key.Value.Y);
                            writer.Write(key.Value.Z);
                        }
                        writer.Write(channel.RotationKeyCount);
                        foreach (var key in channel.RotationKeys) 
                        {
                            writer.Write((float)key.Time);
                            writer.Write(key.Value.X);
                            writer.Write(key.Value.Y);
                            writer.Write(key.Value.Z);
                            writer.Write(key.Value.W); 
                        }
                        writer.Write(channel.ScalingKeyCount);
                        foreach (var key in channel.ScalingKeys)
                        {
                            writer.Write((float)key.Time);
                            writer.Write(key.Value.X);
                            writer.Write(key.Value.Y);
                            writer.Write(key.Value.Z);
                        }
                    }
                }
            }
        }
    }
}
