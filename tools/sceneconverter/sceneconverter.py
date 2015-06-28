import yaml
import sys
import io

# http://stackoverflow.com/questions/21473076/pyyaml-and-unusual-tags
def removeUnityTagAlias(filepath):
    """
    Name:               removeUnityTagAlias()

    Description:        Loads a file object from a Unity textual scene file, which is in a pseudo YAML style, and strips the
                        parts that are not YAML 1.1 compliant. Then returns a string as a stream, which can be passed to PyYAML.
                        Essentially removes the "!u!" tag directive, class type and the "&" file ID directive. PyYAML seems to handle
                        rest just fine after that.

    Returns:                String (YAML stream as string)  


    """
    result = io.StringIO()
    sourceFile = open(filepath, 'r')
    count = 0
    for lineNumber,line in enumerate( sourceFile.readlines() ): 
        if line.startswith('--- !u!'):          
            result.write('--- ' + line.split(' ')[2] + '\n')   # remove the tag, but keep file ID
            print('\rGameObject ' + str(count))
            count += 1
        else: # Just copy the contents...
            result.write(line)
    sourceFile.close()  
    return result.getvalue()

if len(sys.argv) != 2:
    print('sceneconverter.py <inputfile>')
    sys.exit(2)

gameobject_tag = 'tag:unity3d.com,2011:29'
gameobject2_tag = 'tag:unity3d.com,2011:104'

def gameobject_constructor(loader, node):
    pass

print('Importing scene from ' + sys.argv[1])
scene_file = sys.argv[1]
yaml.add_constructor(gameobject_tag, gameobject_constructor)
yaml.add_constructor(gameobject2_tag, gameobject_constructor)
scene_data = removeUnityTagAlias(scene_file)
scene_yaml = yaml.compose_all(scene_data)

component_dict = {}

for entity in scene_yaml:
    #if 'GameObject' in entity:
    print(entity)
