using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SlimDX;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;   // for VectorX type

namespace modelconverter
{
    static class ModelExporter
    {
        struct Submesh
        {
            public string Name;
            public int StartVertex;
            public int StartIndex;
            public int NumVertices;
            public ushort NumIndices;
            public byte Bone;
        }

        struct Bone
        {
            public string Name;
            public Matrix InverseBindPose;
            public Matrix Transform;
            public int Parent;
        }


        private static void BuildSkeleton(
            HashSet<Assimp.Node> skeletonSet,
            List<Bone> bones,
            List<Submesh> submeshes,
            int parent,
            Assimp.Node current)
        {
            if (!skeletonSet.Contains(current))
            {
                return;
            }

            Bone bone = new Bone
            {
                Name = current.Name,
                Transform = new Matrix
                {
                    M11 = current.Transform.A1,
                    M12 = current.Transform.A2,
                    M13 = current.Transform.A3,
                    M14 = current.Transform.A4,
                    M21 = current.Transform.B1,
                    M22 = current.Transform.B2,
                    M23 = current.Transform.B3,
                    M24 = current.Transform.B4,
                    M31 = current.Transform.C1,
                    M32 = current.Transform.C2,
                    M33 = current.Transform.C3,
                    M34 = current.Transform.C4,
                    M41 = current.Transform.D1,
                    M42 = current.Transform.D2,
                    M43 = current.Transform.D3,
                    M44 = current.Transform.D4
                },
                Parent = parent
            };

            bones.Add(bone);
            var boneId = bones.Count - 1;

            foreach (var im in current.MeshIndices)
            {
                var mesh = submeshes[im];
                mesh.Bone = (byte)boneId;
            }

            foreach (var child in current.Children)
            {
                BuildSkeleton(skeletonSet, bones, submeshes, boneId, child);
            }
        }

        public static void Export(Assimp.Scene scene, ProgramOptions options)
        {
            // TODO
            List<Bone> bones = new List<Bone>();
            List<Submesh> submeshes = new List<Submesh>();

            List<Vector3> positions = new List<Vector3>();
            List<Vector3> normals = new List<Vector3>();
            List<Vector3> tangents = new List<Vector3>();
            List<Vector3> bitangents = new List<Vector3>();
            List<Vector2> texcoords = new List<Vector2>();
            List<float[]> boneWeights = new List<float[]>();
            List<sbyte[]> boneIds = new List<sbyte[]>();
            List<ushort> indices = new List<ushort>();

            for (int i = 0; i < boneIds.Count; ++i)
            {
                boneWeights[i] = new float[4] { 0.0f, 0.0f, 0.0f, 0.0f };
                boneIds[i] = new sbyte[4] { -1, -1, -1, -1 };
            }

            // all nodes that are part of the skeleton
            HashSet<Assimp.Node> skeletonSet = new HashSet<Assimp.Node>();
            bool isSkinned = false;
            bool hasMoreThan4Weights = false;

            // Count number of vertices, indices
            int nbvertex = 0, nbindex = 0;
            foreach (var mesh in scene.Meshes)
            {
                nbvertex += mesh.VertexCount;
                nbindex += mesh.FaceCount * 3;
            }

            positions.Capacity = nbvertex;
            normals.Capacity = nbvertex;
            tangents.Capacity = nbvertex;
            bitangents.Capacity = nbvertex;
            texcoords.Capacity = nbvertex;
            indices.Capacity = nbindex;


            // wow
            // such clusterfuck
            // very bullshit
            int vertexbase = 0, indexbase = 0;
            foreach (var mesh in scene.Meshes)
            {
                Console.WriteLine("{0}", mesh);
                // find matching node in hierarchy
                var node = scene.RootNode.FindNode(mesh.Name);
                if (node != null)
                {
                    AddNodes(skeletonSet, node);
                }

                var numIndices = mesh.FaceCount * 3;
                if (numIndices > ushort.MaxValue)
                {
                    throw new InvalidOperationException("Too many indices (" + numIndices.ToString() + " > " + ushort.MaxValue.ToString() + ")");
                }

                if (mesh.BoneCount > 4)
                {
                    hasMoreThan4Weights = true;
                }

                submeshes.Add(new Submesh
                {
                    StartVertex = vertexbase,
                    StartIndex = indexbase,
                    NumVertices = mesh.VertexCount,
                    NumIndices = (ushort)numIndices,
                });

                //Console.WriteLine("Submesh SV:" + vertexbase + ",SI:" + indexbase + ",NV:" + mesh.VertexCount + ",NI:" + numIndices);

                for (int i = 0; i < mesh.VertexCount; ++i)
                {
                    positions.Add(new Vector3(mesh.Vertices[i].X, mesh.Vertices[i].Y, mesh.Vertices[i].Z));
                    normals.Add(new Vector3(mesh.Normals[i].X, mesh.Normals[i].Y, mesh.Normals[i].Z));
                    if (mesh.Tangents.Count > 0)
                    {
                        tangents.Add(new Vector3(mesh.Tangents[i].X, mesh.Tangents[i].Y, mesh.Tangents[i].Z));
                        bitangents.Add(new Vector3(mesh.BiTangents[i].X, mesh.BiTangents[i].Y, mesh.BiTangents[i].Z));
                        texcoords.Add(new Vector2(mesh.TextureCoordinateChannels[0][i].X, mesh.TextureCoordinateChannels[0][i].Y));
                    }
                    else
                    {
                        tangents.Add(new Vector3());
                        bitangents.Add(new Vector3());
                        texcoords.Add(new Vector2());
                    }
                }
                indices.AddRange(Array.ConvertAll(mesh.GetShortIndices(), x => (ushort)x));

                foreach (var bone in mesh.Bones)
                {
                    // mark the nodes that are associated to a bone
                    isSkinned = true;
                    var boneNode = scene.RootNode.FindNode(bone.Name);
                    Debug.Assert(boneNode != null);
                    AddNodes(skeletonSet, boneNode);
                }

                vertexbase += mesh.VertexCount;
                indexbase += numIndices;
            }

            if (hasMoreThan4Weights)
            {
                Console.Error.WriteLine("Warning: some vertices of the model have more than 4 bone weights. Some weights will be discarded");
            }

            // build skeleton
            BuildSkeleton(skeletonSet, bones, submeshes, 255, scene.RootNode);

            if (bones.Count > byte.MaxValue)
            {
                throw new InvalidOperationException("Too many bones (" + bones.Count.ToString() + " > " + byte.MaxValue.ToString() + ")");
            }

            if (isSkinned)
            {
                vertexbase = 0;
                for (int i = 0; i < nbvertex; ++i)
                {
                    boneIds.Add(new sbyte[4]);
                    boneWeights.Add(new float[4]);
                }
                foreach (var mesh in scene.Meshes)
                {
                    foreach (var assbone in mesh.Bones)
                    {
                        // C# power
                        var boneid = bones.FindIndex(b => b.Name == assbone.Name);
                        var bone = bones[boneid];
                        bone.InverseBindPose = new Matrix
                        {
                            M11 = assbone.OffsetMatrix.A1,
                            M12 = assbone.OffsetMatrix.A2,
                            M13 = assbone.OffsetMatrix.A3,
                            M14 = assbone.OffsetMatrix.A4,
                            M21 = assbone.OffsetMatrix.B1,
                            M22 = assbone.OffsetMatrix.B2,
                            M23 = assbone.OffsetMatrix.B3,
                            M24 = assbone.OffsetMatrix.B4,
                            M31 = assbone.OffsetMatrix.C1,
                            M32 = assbone.OffsetMatrix.C2,
                            M33 = assbone.OffsetMatrix.C3,
                            M34 = assbone.OffsetMatrix.C4,
                            M41 = assbone.OffsetMatrix.D1,
                            M42 = assbone.OffsetMatrix.D2,
                            M43 = assbone.OffsetMatrix.D3,
                            M44 = assbone.OffsetMatrix.D4
                        };

                        foreach (var weight in assbone.VertexWeights)
                        {
                            for (int iw = 0; iw < 4; ++iw)
                            {
                                if (boneIds[vertexbase + weight.VertexID][iw] == -1)
                                {
                                    boneWeights[vertexbase + weight.VertexID][iw] = weight.Weight;
                                    break;
                                }
                            }
                        }
                    }
                    vertexbase += mesh.VertexCount;
                }
            }

            // write to file
            /*var path = Path.Combine(options.OutputDirectory, options.ModelName + ".model");
            Console.WriteLine("Writing master model " + path + "...");
            using (var streamOut = new FileStream(path, FileMode.Create, FileAccess.Write))
            //using (var gzipStream = new GZipStream(streamOut, CompressionLevel.Fastest))
            {
                BinaryWriter writer = new BinaryWriter(streamOut);
                writer.Write(submeshes.Count);
                foreach (var sm in submeshes)
                {
                    writer.Write(sm.StartVertex);
                    writer.Write(sm.StartIndex);
                    writer.Write(sm.NumVertices);
                    writer.Write(sm.NumIndices);
                    writer.Write((byte)sm.Bone);
                }
                writer.Write((byte)bones.Count);
                foreach (var b in bones)
                {
                    writer.Write(b.Name);

                    writer.Write(b.Transform.M11);
                    writer.Write(b.Transform.M12);
                    writer.Write(b.Transform.M13);
                    writer.Write(b.Transform.M14);
                    writer.Write(b.Transform.M21);
                    writer.Write(b.Transform.M22);
                    writer.Write(b.Transform.M23);
                    writer.Write(b.Transform.M24);
                    writer.Write(b.Transform.M31);
                    writer.Write(b.Transform.M32);
                    writer.Write(b.Transform.M33);
                    writer.Write(b.Transform.M34);
                    writer.Write(b.Transform.M41);
                    writer.Write(b.Transform.M42);
                    writer.Write(b.Transform.M43);
                    writer.Write(b.Transform.M44);

                    writer.Write(b.InverseBindPose.M11);
                    writer.Write(b.InverseBindPose.M12);
                    writer.Write(b.InverseBindPose.M13);
                    writer.Write(b.InverseBindPose.M14);
                    writer.Write(b.InverseBindPose.M21);
                    writer.Write(b.InverseBindPose.M22);
                    writer.Write(b.InverseBindPose.M23);
                    writer.Write(b.InverseBindPose.M24);
                    writer.Write(b.InverseBindPose.M31);
                    writer.Write(b.InverseBindPose.M32);
                    writer.Write(b.InverseBindPose.M33);
                    writer.Write(b.InverseBindPose.M34);
                    writer.Write(b.InverseBindPose.M41);
                    writer.Write(b.InverseBindPose.M42);
                    writer.Write(b.InverseBindPose.M43);
                    writer.Write(b.InverseBindPose.M44);

                    writer.Write((byte)b.Parent);
                }
                writer.Write(positions.Count);
                writer.Write(indices.Count);
                writer.Write(isSkinned ? (byte)1 : (byte)0);

                foreach (var pos in positions)
                {
                    writer.Write(pos.X);
                    writer.Write(pos.Y);
                    writer.Write(pos.Z);
                }

                foreach (var n in normals)
                {
                    writer.Write(n.X);
                    writer.Write(n.Y);
                    writer.Write(n.Z);
                }

                foreach (var t in tangents)
                {
                    writer.Write(t.X);
                    writer.Write(t.Y);
                    writer.Write(t.Z);
                }

                foreach (var b in bitangents)
                {
                    writer.Write(b.X);
                    writer.Write(b.Y);
                    writer.Write(b.Z);
                }

                foreach (var tex in texcoords)
                {
                    writer.Write(tex.X);
                    writer.Write(tex.Y);
                }

                if (isSkinned)
                {
                    foreach (var bid in boneIds)
                    {
                        writer.Write(bid[0]);
                        writer.Write(bid[1]);
                        writer.Write(bid[2]);
                        writer.Write(bid[3]);
                    }
                    foreach (var bw in boneWeights)
                    {
                        writer.Write(bw[0]);
                        writer.Write(bw[1]);
                        writer.Write(bw[2]);
                        writer.Write(bw[3]);
                    }
                }

                foreach (var ix in indices)
                {
                    writer.Write(ix);
                }
            }*/

            // V3 exporter
            var path = Path.Combine(options.OutputDirectory, options.ModelName + ".mesh");
            Console.WriteLine("Writing mesh " + path + "...");

            using (var streamOut = new FileStream(path, FileMode.Create, FileAccess.Write))
            //using (var gzipStream = new GZipStream(streamOut, CompressionLevel.Fastest))
            {
                BinaryWriter writer = new BinaryWriter(streamOut);

                writer.Write((byte)3);      // V3
                writer.Write((byte)1);      // Layout type 1
                writer.Write((ushort)submeshes.Count);
                writer.Write(positions.Count);
                writer.Write(indices.Count);
                
                foreach (var sm in submeshes)
                {
                    writer.Write(sm.StartVertex);
                    writer.Write(sm.StartIndex);
                    writer.Write(sm.NumVertices);
                    writer.Write(sm.NumIndices);
                }

                for (int i = 0; i < positions.Count; ++i) 
                {
                    writer.Write(positions[i].X);
                    writer.Write(positions[i].Y);
                    writer.Write(positions[i].Z);
                    writer.Write(normals[i].X);
                    writer.Write(normals[i].Y);
                    writer.Write(normals[i].Z);
                    writer.Write(tangents[i].X);
                    writer.Write(tangents[i].Y);
                    writer.Write(tangents[i].Z);
                    writer.Write(bitangents[i].X);
                    writer.Write(bitangents[i].Y);
                    writer.Write(bitangents[i].Z);
                    writer.Write(texcoords[i].X);
                    writer.Write(texcoords[i].Y);
                }

                foreach (var ix in indices)
                {
                    writer.Write(ix);
                }
            }
        }

        private static void AddNodes(HashSet<Assimp.Node> skeletonSet, Assimp.Node node)
        {
            while (node != null)
            {
                skeletonSet.Add(node);
                node = node.Parent;
            }
        }

    }
}
