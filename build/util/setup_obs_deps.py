
import os
import sys
import shutil

from os import listdir, mkdir, path


# Does not support recursive copy.
def copy_files_in_directory(src_dir, dest_dir, overwrite_if_exist=True, pred=None):
    if not path.exists(dest_dir):
        os.makedirs(dest_dir)

    src_files = listdir(src_dir)
    for filename in src_files:
        src_path = path.join(src_dir, filename)
        if(path.isdir(src_path)):
            target_dir = path.join(dest_dir, filename)
            copy_files_in_directory(src_path, target_dir)
        if not path.isfile(src_path):
            continue
        if pred and not pred(src_path):
            continue
        dest_path = path.join(dest_dir, filename)
        if not overwrite_if_exist and path.exists(dest_path):
            print(dest_path + ' already exists; skip')
            continue
        shutil.copyfile(src_path, dest_path)
        print(dest_path + ' is deployed')


def setup_precompiled_dlls(cur_dir, build_mode,build_platform):
    if build_platform.lower() == 'win32':
       deps_dir = path.join(cur_dir, '../../obs/dependencies2019/win32/bin')
    else:
       deps_dir = path.join(cur_dir, '../../obs/dependencies2019/win64/bin')
    target_dir = path.join(cur_dir, '../',build_platform, build_mode)
    obs_win32_pred = lambda src_path: src_path.lower().endswith('.dll')
    copy_files_in_directory(deps_dir, target_dir, True, obs_win32_pred)

    # obs-plugins/win-capture's datas use obs-studio offical binaries
    graphics_hook_dir = path.join(cur_dir, '../../obs/obs-studio-original-bin/graphics-hook')
    target_dir = path.join(cur_dir, '..', build_platform,build_mode, 'data/plugins/win-capture')
    copy_files_in_directory(graphics_hook_dir, target_dir)
    
    vulkan32_path = path.join(cur_dir, '../../obs/obs-studio/plugins/win-capture/graphics-hook/obs-vulkan32.json')
    vulkan64_path = path.join(cur_dir, '../../obs/obs-studio/plugins/win-capture/graphics-hook/obs-vulkan64.json')
    target32_path = path.join(target_dir, 'obs-vulkan32.json')
    target64_path = path.join(target_dir, 'obs-vulkan64.json')
    shutil.copyfile(vulkan32_path, target32_path)
    shutil.copyfile(vulkan64_path, target64_path)

    get_graphics_offsets_dir = path.join(cur_dir, '../../obs/obs-studio-original-bin/get-graphics-offsets')
    copy_files_in_directory(get_graphics_offsets_dir, target_dir)

    inject_helper_dir = path.join(cur_dir, '../../obs/obs-studio-original-bin/inject-helper')
    copy_files_in_directory(inject_helper_dir, target_dir)

    d3d_compiler_dir = path.join(cur_dir, '../../obs/obs-studio-original-bin/d3dcompiler')
    target_dir = path.join(cur_dir, '../',build_platform,build_mode)
    copy_files_in_directory(d3d_compiler_dir, target_dir)

    print('pre-compiled deps all settled')


def setup_libobs_effect_data(cur_dir, build_mode):
    deps_dir = path.join(cur_dir, '../../obs/obs-studio/libobs/data')
    data_dir = path.join(cur_dir, '../' + build_mode, 'data/libobs')
    if path.exists(data_dir):
        if build_mode.lower() == 'debug':
            print('libobs data files already exist. We do nothing next')
            return
        else:
            print('Removing existing libobs effect data')
            shutil.rmtree(data_dir)
    shutil.copytree(deps_dir, data_dir)
    print('libobs data files all settled')


def setup_plugin_obs_transitions_data(cur_dir, build_mode):
    print('Setup data files for obs-transitions!')

    in_release_mode = not(build_mode.lower() == 'debug')

    src_dir = path.join(cur_dir, '../../obs/obs-studio/plugins/obs-transitions/data')
    dest_dir = path.join(cur_dir, '..', build_mode, 'data/plugins/obs-transitions')
    copy_files_in_directory(src_dir, dest_dir, in_release_mode)

    src_luma_dir = path.join(src_dir, 'luma_wipes')
    dest_luma_dir = path.join(dest_dir, 'luma_wipes')
    copy_files_in_directory(src_luma_dir, dest_luma_dir, in_release_mode)

    print('data files for obs-transition are deployed')


def setup_plugin_obs_filters_data(cur_dir, build_mode):
    print('Setup data files for obs-filters!')
    src_dir = path.join(cur_dir, '../../obs/obs-studio/plugins/obs-filters/data')
    dest_dir = path.join(cur_dir, '..', build_mode, 'data/plugins/obs-filters')
    if not path.exists(dest_dir):
        mkdir(dest_dir)
    effect_files = [f for f in listdir(src_dir) if path.isfile(path.join(src_dir, f))]
    for ef in effect_files:
        src_path = path.join(src_dir, ef)
        dest_path = path.join(dest_dir, ef)
        if path.exists(dest_path) and build_mode.lower() == 'debug':
            print(dest_path + ' already exists; skip')
            continue
        shutil.copyfile(src_path, dest_path)
        print(ef + ' is deployed')

    src_luts_dir = path.join(src_dir, 'LUTs')
    dest_luts_dir = path.join(dest_dir, 'LUTs')
    if path.exists(dest_luts_dir):
        if build_mode.lower() == 'debug':
            print('obs-filters LUTs already exists. Skip it.')
            return
        else:
            print('Removing existing obs-filters LUTs')
            shutil.rmtree(dest_luts_dir)
    shutil.copytree(src_luts_dir, dest_luts_dir)
    print('obs-filters LUTs is deployed')
    
def setup_cef_sdk(cur_dir, build_mode,build_platform):
    print('setup cef sdk files...')
    deps_dir = path.join(cur_dir, '../../cef/cef_core/', build_platform, build_mode)
    target_dir = path.join(cur_dir, '../', build_platform ,build_mode, 'plugins')
    copy_files_in_directory(deps_dir, target_dir)
    print('cef bin deployed')

    res_dir = path.join(cur_dir, '../../cef/cef_core/',build_platform,'Resources')
    copy_files_in_directory(res_dir, target_dir)
    print('cef res deployed')

def setup_zlib_dll(cur_dir, build_mode,build_platform):
    if build_platform.lower() == 'win32':
       zlib_path = path.join(cur_dir, '../../obs/dependencies2019/win32/bin/zlib.dll')
    else:
       zlib_path = path.join(cur_dir, '../../obs/dependencies2019/win64/bin/zlib.dll')
       
    target_path = path.join(cur_dir, '..',build_platform,build_mode,'data','plugins','obs-ffmpeg','zlib.dll')
    print('target_path:' + target_path)
    shutil.copyfile(zlib_path, target_path)
    print('zlib in obs-ffmpeg deployed')
    
    
def setup_plugin_obs_dll(cur_dir, build_mode,build_platform):
    print('setup_plugin_obs_dll files...')
    deps_dir = path.join(cur_dir, '..', build_mode,build_platform)
    target_dir = path.join(cur_dir, '..', build_platform ,build_mode)
    copy_files_in_directory(deps_dir, target_dir)
    print('setup_plugin_obs_dll deployed')
    
    
def main():
    build_platform = sys.argv[2]
    build_mode = sys.argv[1]
    cur_dir = path.dirname(sys.argv[0])  # the folder that contains this script.
    setup_precompiled_dlls(cur_dir, build_mode,build_platform)
    #setup_libobs_effect_data(cur_dir, build_mode)
    #setup_plugin_obs_transitions_data(cur_dir, build_mode)
    #setup_plugin_obs_filters_data(cur_dir, build_mode)
    #setup_plugin_obs_dll(cur_dir, build_mode,build_platform)
    setup_cef_sdk(cur_dir, build_mode,build_platform)
    setup_zlib_dll(cur_dir, build_mode,build_platform)


if __name__ == '__main__':
    main()
