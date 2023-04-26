
import os
import sys
import shutil
from os import listdir, mkdir, path

TABOO_LIST = ('ffplay.exe', 'ffprobe.exe')

def setup_components(build_mode, target_dir, deps_dir, pred):
    for dep_file in os.listdir(deps_dir):
        if not (pred(dep_file)):
            continue
        target_file_path = os.path.join(target_dir, dep_file)
        dep_file_path = os.path.join(deps_dir, dep_file)
        print('deploying ' + dep_file)
        if(os.path.isdir(dep_file_path)):
            if not os.path.exists(target_file_path):
                os.makedirs(target_file_path)
            setup_components(build_mode, target_file_path, dep_file_path, pred)
        else:
            shutil.copyfile(dep_file_path, target_file_path)

def setup_feature_file(cur_dir, build_mode,build_platform):
    feature_src_path = os.path.join(cur_dir, '../features')
    target_path = os.path.join(cur_dir, '..', build_platform ,build_mode, 'features')
    print('deploying feature file')
    shutil.copyfile(feature_src_path, target_path)

def setup_animation_deps(cur_dir, build_mode,build_platform):
    feature_src_path = os.path.join(cur_dir, '../bililive_animation.pak')
    target_path = os.path.join(cur_dir, '..', build_platform,build_mode, 'bililive_animation.pak')
    print('deploying bililive_animation.pak')
    shutil.copyfile(feature_src_path, target_path)

def setup_agora_sdk(cur_dir, build_mode,build_platform):
    if build_platform.lower() == 'win32':
       deps_dir = os.path.join(cur_dir, '../../third_party/agora/dll/Win32')
    else:
       deps_dir = os.path.join(cur_dir, '../../third_party/agora/dll/x64')
    target_dir = os.path.join(cur_dir, '../', build_platform ,build_mode)
    setup_components(build_mode, target_dir, deps_dir, lambda x: True)
    print('agora sdk all settled')
    
def setup_rtc_colive(cur_dir, build_mode,build_platform):
    if build_platform.lower() == 'win32':
       deps_dir = os.path.join(cur_dir, '../../third_party/webrtc/depends/openh264/bin/Win32')
    else:
       deps_dir = os.path.join(cur_dir, '../../third_party/webrtc/depends/openh264/bin/x64')
    target_dir = os.path.join(cur_dir, '../', build_platform, build_mode)
    setup_components(build_mode, target_dir+'/plugins', deps_dir, lambda name: name.endswith('.dll'))
    print('rtc_colive all settled')	

def setup_secret_sdk(cur_dir, build_mode,build_platform):
    if build_platform.lower() == 'win32':
       deps_dir = os.path.join(cur_dir, '../../bililive/secret/deps/openssl-1.0.2h/Win32/bin')
    else:
       deps_dir = os.path.join(cur_dir, '../../bililive/secret/deps/openssl-1.0.2h/x64/bin')
    target_dir = os.path.join(cur_dir, '../', build_platform ,build_mode)
    setup_components(build_mode, target_dir, deps_dir, lambda x: True)
    print('setup_secret_sdk settled')

def setup_preset_material(cur_dir, build_mode,build_platform):
    deps_dir = os.path.join(cur_dir, '../../preset_material')
    target_dir = os.path.join(cur_dir, '../', build_platform ,build_mode, 'preset_material')

    if not os.path.exists(deps_dir):
        os.makedirs(deps_dir)

    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    setup_components(build_mode, target_dir, deps_dir, lambda x: True)
    print('deploying preset_material')
    
def setup_echo_test_file(cur_dir, build_mode,build_platform):
    deps_dir = os.path.join(cur_dir, '../../third_party/echo_test_file')
    target_dir = os.path.join(cur_dir, '../', build_platform,build_mode)
    setup_components(build_mode, target_dir, deps_dir, lambda x: True)
    print('echo test file')    
    
def main():
    build_platform = sys.argv[2]
    build_mode = sys.argv[1]
    cur_dir = os.path.dirname(sys.argv[0])  # the folder that contains this script.

    setup_feature_file(cur_dir, build_mode,build_platform)
    setup_animation_deps(cur_dir, build_mode,build_platform)
    setup_agora_sdk(cur_dir, build_mode,build_platform)
    setup_rtc_colive(cur_dir,build_mode,build_platform)
    setup_secret_sdk(cur_dir,build_mode,build_platform)
    setup_preset_material(cur_dir, build_mode,build_platform)
    setup_echo_test_file(cur_dir, build_mode,build_platform)

if __name__ == '__main__':
    main()
