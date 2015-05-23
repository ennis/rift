using UnityEngine;
using System.Collections;
using UnityEditor;
using System.IO;
using System.Collections.Generic;

public class RiftSceneExporter
{
    enum ObjectType : short
    {
        GameObject = 0,
        Material
    }

    enum ComponentTag : short
    {
        Transform,
        Mesh,
        End = -1,
    }

    private static string GetExportedMeshPath(string outputDirectory, Mesh mesh)
    {
        var assetPath = AssetDatabase.GetAssetPath(mesh);
        return Path.Combine(outputDirectory, mesh.name + ".mesh");
    }

    private static string GetExportedTexturePath(string outputDirectory, Texture tex)
    {
        var assetPath = AssetDatabase.GetAssetPath(tex);
        return Path.Combine(outputDirectory, Path.ChangeExtension(Path.GetFileName(assetPath), Path.GetExtension(assetPath).ToLower()));
    }

    private static void ExportMaterial(BinaryWriter streamOut, Material mat)
    {
        // this is supposed to be a Standard material
        streamOut.Write(mat.name);
        streamOut.Write(mat.mainTexture.name);
    }
    
    [MenuItem("Tools/Export scene to Rift")]
    private static void NewMenuOption()
    {
        Debug.Log("Clicked!");

        // Choose output directory
        var dir = EditorUtility.OpenFolderPanel("Select output directory", "", "");
        // create output directory
        var dirInfo = Directory.CreateDirectory(dir);
        // create output file for main scene file
        using (var streamOut = new FileStream(Path.Combine(dir, "scene.bin"), FileMode.Create, FileAccess.Write))
        {
            var w = new BinaryWriter(streamOut);
            // Scene file version 1
            w.Write((byte)1);

            var objs = Object.FindObjectsOfType<GameObject>();
            int count = 0;
            int max = objs.Length;

            var meshesToConvert = new HashSet<Mesh>();
            var materialsToConvert = new HashSet<Material>();
            var texturesToConvert = new HashSet<Texture>();

            //=================================================
            //=================================================
            // GAME OBJECTS
            //=================================================
            //=================================================
            foreach (var obj in Object.FindObjectsOfType<GameObject>())
            {
                EditorUtility.DisplayProgressBar("Exporting game objects...", obj.name, (float)count / (float)max);
                var transform = obj.GetComponent<Transform>();
                //if (transform != null)
                //    Debug.Log(transform.position);
                w.Write((short)ObjectType.GameObject);
                // object name
                w.Write(obj.name);
                // object ID
                w.Write(obj.GetInstanceID());
                // parent ID
                w.Write((short)ComponentTag.Transform);
                if (transform.parent != null)
                    w.Write(transform.parent.gameObject.GetInstanceID());
                else
                    w.Write(-1);
                w.Write(transform.localPosition.x);
                w.Write(transform.localPosition.y);
                w.Write(transform.localPosition.z);
                w.Write(transform.localRotation.x);
                w.Write(transform.localRotation.y);
                w.Write(transform.localRotation.z);
                w.Write(transform.localRotation.w);
                w.Write(transform.localScale.x);
                w.Write(transform.localScale.y);
                w.Write(transform.localScale.z);
                // mesh component
                var meshFilter = obj.GetComponent<MeshFilter>();
                if (meshFilter != null)
                {
                    w.Write((short)ComponentTag.Mesh);
                    w.Write(meshFilter.sharedMesh.name);
                    meshesToConvert.Add(meshFilter.sharedMesh);
                    // materials
                    var renderer = obj.GetComponent<Renderer>();
                    var mat = renderer.sharedMaterials;
                    w.Write(mat.Length);
                    foreach (var m in mat)
                    {
                        w.Write(m.name);
                        materialsToConvert.Add(m);
                        Debug.Log("Material: " + m.name);
                        if (m.mainTexture)
                        {
                            texturesToConvert.Add(m.mainTexture);
                        }
                    }   
                }             

                // end gameobject
                w.Write((short)ComponentTag.End);
                // output transform
                ++count;
                Object prefab = PrefabUtility.GetPrefabObject(obj);
                if (prefab != null)
                {
                    Debug.Log(obj.name + " is a prefab instance: " + prefab.name);
                }
            }

            //=================================================
            //=================================================
            // MESHES
            //=================================================
            //=================================================
            EditorUtility.ClearProgressBar();
            count = 0;
            max = meshesToConvert.Count;
            foreach (var mesh in meshesToConvert)
            {
                EditorUtility.DisplayProgressBar("Processing meshes...", mesh.name, (float)count / (float)max);
                var targetPath = GetExportedMeshPath(dir, mesh);
                Directory.CreateDirectory(Path.GetDirectoryName(targetPath));
                RiftMeshExporter.ExportMesh(targetPath, mesh);
                ++count;
            }

            //=================================================
            //=================================================
            // TEXTURES
            //=================================================
            //=================================================
            EditorUtility.ClearProgressBar();
            count = 0;
            max = texturesToConvert.Count;
            foreach (var tex in texturesToConvert)
            {
                EditorUtility.DisplayProgressBar("Processing textures...", tex.name, (float)count / (float)max);
                var texFile = AssetDatabase.GetAssetPath(tex);
                var targetPath = GetExportedTexturePath(dir, tex);
                File.Copy(texFile, targetPath, true);
                ++count;
            }

            EditorUtility.ClearProgressBar();

            //=================================================
            //=================================================
            // MATERIALS
            //=================================================
            //=================================================   
            EditorUtility.ClearProgressBar();
            count = 0;
            max = materialsToConvert.Count;
            foreach (var mat in materialsToConvert)
            {
                EditorUtility.DisplayProgressBar("Processing materials...", mat.name, (float)count / (float)max);
                var targetPath = Path.Combine(dir, mat.name + ".mat");
                using (FileStream fs = new FileStream(targetPath, FileMode.Create, FileAccess.Write))
                {
                    BinaryWriter bin = new BinaryWriter(fs);
                    // standard shader
                    bin.Write("shaders/standard");
                    if (mat.mainTexture)
                    {
                        bin.Write("mainTexture");
                        bin.Write(mat.mainTexture.name);
                    }
                }
                ++count;
            }

            EditorUtility.ClearProgressBar();
        }
    }

}