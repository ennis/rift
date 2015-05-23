using UnityEngine;
using System.Collections;
using System.IO;
using System;

public static class RiftMeshExporter
{
    public static void ExportMesh(string path, Mesh mesh)
    {
        // V3 exporter
        Debug.Log("Writing mesh " + path + "...");

        using (var streamOut = new FileStream(path, FileMode.Create, FileAccess.Write))
        //using (var gzipStream = new GZipStream(streamOut, CompressionLevel.Fastest))
        {
            BinaryWriter writer = new BinaryWriter(streamOut);

            writer.Write((byte)3);      // V3
            // layout type 5 (static mesh w/o bitangents)
            writer.Write((byte)5);
            writer.Write((ushort)mesh.subMeshCount);
            writer.Write(mesh.vertexCount);
            writer.Write(mesh.triangles.Length);

            int startIndex = 0;
            for (int i = 0; i < mesh.subMeshCount; i++)
            {
                var triangles = mesh.GetTriangles(i);
                writer.Write(0);
                writer.Write(startIndex);
                writer.Write(mesh.vertexCount);
                writer.Write(triangles.Length);
            }

            for (int i = 0; i < mesh.vertexCount; i++) 
            {
                writer.Write(mesh.vertices[i].x);
                writer.Write(mesh.vertices[i].y);
                writer.Write(mesh.vertices[i].z);
                writer.Write(mesh.normals[i].x);
                writer.Write(mesh.normals[i].y);
                writer.Write(mesh.normals[i].z);
                writer.Write(mesh.tangents[i].x);
                writer.Write(mesh.tangents[i].y);
                writer.Write(mesh.tangents[i].z);
                // TODO multi-uv
                writer.Write(mesh.uv[i].x);
                writer.Write(mesh.uv[i].y);
            }

            // write indices

            foreach (var index in mesh.triangles)
            {
                writer.Write((ushort)index);
            }
        }
    }
}
