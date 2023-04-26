
import os
import sys
import xml.dom.minidom
import xml.etree.ElementTree as ET

from os import listdir, mkdir, path

def check_one_vcxproj_file(vcxproj_path, save_path):
    if path.isfile(vcxproj_path):
        if path.splitext(vcxproj_path.lower())[1] == '.vcxproj':
            print('\t Check .vcxproj file -> ' + vcxproj_path)

            need_edit = False

            ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')

            ns = {'vs': 'http://schemas.microsoft.com/developer/msbuild/2003'}

            vcxproj = ET.parse(vcxproj_path)

            cls = vcxproj.findall('./vs:ItemDefinitionGroup', ns)
            # print cls

            for cl in cls:
                print('\t\t Platform -> ' + cl.attrib['Condition'])

                # delete window sdk 8.0 includes
                remove_includes = [r"C:\Program Files (x86)\Windows Kits\8.0\Include\shared",
                                   r"C:\Program Files (x86)\Windows Kits\8.0\Include\um",
                                   r"C:\Program Files (x86)\Windows Kits\8.0\Include\winrt",
                                   r"C:/Program Files (x86)/Windows Kits/8.0/Include/shared",
                                   r"C:/Program Files (x86)/Windows Kits/8.0/Include/um",
                                   r"C:/Program Files (x86)/Windows Kits/8.0/Include/winrt",
                                   r"$(VSInstallDir)\VC\atlmfc\include",
                                   r"$(VSInstallDir)/VC/atlmfc/include"]
                include = cl.find('./vs:ClCompile/vs:AdditionalIncludeDirectories', ns)
                if not include is None:
                    print('\t\t\t Old Include -> ' + include.text)
                    include_list = include.text.split(';')
                    for del_inc in remove_includes:
                        if include_list.count(del_inc) != 0:
                            include_list.remove(del_inc)
                            need_edit = True
                    include.text = ";".join(include_list)
                    print('\t\t\t New Include -> ' + include.text)

                # delete window sdk 8.0 res includes
                res_include = cl.find('./vs:ResourceCompile/vs:AdditionalIncludeDirectories', ns)
                if not res_include is None:
                    print('\t\t\t Old Res Include -> ' + res_include.text)
                    res_include_list = res_include.text.split(';')
                    for del_inc in remove_includes:
                        if res_include_list.count(del_inc) != 0:
                            res_include_list.remove(del_inc)
                            need_edit = True
                    res_include.text = ";".join(res_include_list)
                    print('\t\t\t New Res Include -> ' + res_include.text)

                # delete window sdk 8.0 libs
                remove_lib_dirs = [r"C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x86",
                                   r"C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64",
                                   r"C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86",
                                   r"C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x64"]
                lib_dir = cl.find('./vs:Lib/vs:AdditionalLibraryDirectories', ns)
                if not lib_dir is None:
                    print('\t\t\t Old Lib Dir -> ' + lib_dir.text)
                    lib_dir_list = lib_dir.text.split(';')
                    for del_lib_dir in remove_lib_dirs:
                        if lib_dir_list.count(del_lib_dir) != 0:
                            lib_dir_list.remove(del_lib_dir)
                            need_edit = True
                    lib_dir.text = ";".join(lib_dir_list)
                    print('\t\t\t New Lib Dir -> ' + lib_dir.text)

                # delete link libs
                link_dir = cl.find('./vs:Link/vs:AdditionalLibraryDirectories', ns)
                if not link_dir is None:
                    print('\t\t\t Old Link Dir -> ' + link_dir.text)
                    link_dir_list = link_dir.text.split(';')
                    for del_link_dir in remove_lib_dirs:
                        if link_dir_list.count(del_link_dir) != 0:
                            link_dir_list.remove(del_link_dir)
                            need_edit = True
                    link_dir.text = ";".join(link_dir_list)
                    print('\t\t\t New Link Dir -> ' + link_dir.text)

                # add ignore hash_map warning
                add_predef = ["_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS",
                              "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING",
                              "_TIMESPEC_DEFINED",
                              "HAVE_SNPRINTF"]
                predef = cl.find('./vs:ClCompile/vs:PreprocessorDefinitions', ns)
                if not predef is None:
                    print('\t\t\t Old PreDef -> ' + predef.text)
                    predef_list = predef.text.split(';')
                    for add_def in add_predef:
                        if predef_list.count(add_def) == 0:
                            predef_list.insert(len(predef_list) - 1, add_def)
                            need_edit = True
                    predef.text = ";".join(predef_list)
                    print('\t\t\t New PreDef -> ' + predef.text)

                # add ignore variable declare conflict
                add_warns = [4456, 4457, 4458, 4459, 4302]
                warn = cl.find('./vs:ClCompile/vs:DisableSpecificWarnings', ns)
                if not warn is None:
                    print('\t\t\t Old DisWarn -> ' + warn.text)
                    warn_list = warn.text.split(';')
                    for add_warn in add_warns:
                        if warn_list.count(add_warn) == 0:
                            warn_list.insert(len(warn_list) - 1, str(add_warn))
                            need_edit = True
                    warn.text = ";".join(warn_list)
                    print('\t\t\t New DisWarn -> ' + warn.text)

            if need_edit:
                save_dir = os.path.dirname(save_path)
                if not os.path.exists(save_dir):
                    os.makedirs(save_dir)

                vcxproj.write(save_path,
                              "utf-8",
                              "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                              None,
                              "xml")
                print("\t Save .vcxproj file -> " + save_path)
            else:
                print("\t No need to edit -> " + vcxproj_path)

            return need_edit

def check_one_folder(vcxproj_dir, save_dir):
    if not path.exists(vcxproj_dir):
        print('invalid vcxproj_dir.')
        return

    print('vcxproj_dir = \'' + vcxproj_dir + '\'')
    print('save_dir = \'' + save_dir + '\'')
    edit_count = 0
    vcxprojs = listdir(vcxproj_dir)
    for vcxproj_file in vcxprojs:
        vcxproj_path = path.join(vcxproj_dir, vcxproj_file)
        save_path = path.join(save_dir, vcxproj_file)
        if check_one_vcxproj_file(vcxproj_path, save_path):
            edit_count += 1
    print("{0} vcxproj files changed!".format(edit_count))

def srcroot_editrelate_save():
    if len(sys.argv) < 4:
        print('No full params specified! Edit .vcxproj Dir and Save .vcxproj Dir!')
        return

    #cur_dir = os.path.dirname(sys.argv[0])  # the folder that contains this script.
    root_src_dir = sys.argv[1]
    relate_vcxproj_dir = sys.argv[2]
    vcxproj_dir = os.path.join(root_src_dir, relate_vcxproj_dir)
    #dir_name = os.path.basename(vcxproj_dir)
    save_dir = os.path.join(sys.argv[3], relate_vcxproj_dir)

    check_one_folder(vcxproj_dir, save_dir)

def modify_bililvie_strings_vcxproj():
    cur_dir = os.path.dirname(sys.argv[0])
    item_file = os.path.join(cur_dir, 'bililive_strings_invalid_items.list')

    # invalid AdditionalInputs
    remove_includes = []
    for l in open(item_file, 'r'):
        l = l.strip()
        if len(l) == 0:
            continue
        #comment
        if l[0] in ('#', ';'):
            continue

        remove_includes.append(l)

    vcxproj_path = sys.argv[1]
    save_path = sys.argv[2]
    if path.isfile(vcxproj_path):
        if path.splitext(vcxproj_path.lower())[1] == '.vcxproj':
            print('\t Check .vcxproj file -> ' + vcxproj_path)

            need_edit = False

            ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')

            ns = {'vs': 'http://schemas.microsoft.com/developer/msbuild/2003'}

            vcxproj = ET.parse(vcxproj_path)

            cls = vcxproj.findall('./vs:ItemGroup/vs:CustomBuild', ns)
            # print cls

            for cl in cls:
                if cl.attrib['Include'] != '..\\tools\gritsettings\\resource_ids':
                    continue

                print('\t\t CustomBuild Include -> ' + cl.attrib['Include'])

                # delete invalid AdditionalInputs
                AdditionalInputs = cl.findall('./vs:AdditionalInputs', ns)
                for Inputs in AdditionalInputs:
                    print('\t\t Inputs Condition -> ' + Inputs.attrib['Condition'])
                    print('\t\t\t Old Include -> ' + Inputs.text)
                    include_list = Inputs.text.lower().split(';')
                    for del_inc in remove_includes:
                        if include_list.count(del_inc) != 0:
                            include_list.remove(del_inc)
                            need_edit = True
                    Inputs.text = ";".join(include_list)
                    print('\t\t\t New Include -> ' + Inputs.text)

            if need_edit:
                save_dir = os.path.dirname(save_path)
                if not os.path.exists(save_dir):
                    os.makedirs(save_dir)

                vcxproj.write(save_path,
                              "utf-8",
                              "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                              None,
                              "xml")
                print("\t Save .vcxproj file -> " + save_path)
            else:
                print("\t No need to edit -> " + vcxproj_path)

            return need_edit


if __name__ == '__main__':

    # vs2013 v120 to vs2017 v141 [
    #if len(sys.argv) == 4:
    #    srcroot_editrelate_save()
    #elif len(sys.argv) == 2:
    #    if os.path.isdir(sys.argv[1]):
    #        check_one_folder(sys.argv[1], sys.argv[1])
    #    elif os.path.isfile(sys.argv[1]):
    #        check_one_vcxproj_file(sys.argv[1], sys.argv[1])
    # ]

    # vs2017 v141 to vs2019 v142 [
    modify_bililvie_strings_vcxproj()
    # ]