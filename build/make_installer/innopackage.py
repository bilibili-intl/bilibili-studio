#coding:utf-8
import os
import sys
import subprocess
import uuid
import shutil

root_dir = os.path.dirname(os.path.abspath(__file__))

def remove_path(p):
    if not os.path.exists(p):
        return
    if os.path.isfile(p):
        os.remove(p)
        return
    
    for f in os.listdir(p):
        remove_path(os.path.join(p, f))
    os.rmdir(p)

def make(input_path, version, tmp_path,output_path,build_platform):
    try:
        tmp_dir = os.path.join(tmp_path, uuid.uuid1().hex)
        tmp_output_path = os.path.join(tmp_dir, 'setup.exe')
        new_env = os.environ.copy()
        new_env['INNO_INPUT_DIR'] = input_path.encode('gbk')
        new_env['INNO_LICENSE_FILE'] = os.path.join(root_dir, 'bililivehime\\res\\license.txt').encode('gbk')
        new_env['INNO_OUTPUT_FILEICON'] = os.path.join(root_dir, 'bililivehime\\res\\inst_icon.ico').encode('gbk')
        new_env['INNO_SETUP_RES_DIR'] = os.path.join(root_dir, 'bililivehime\\res').encode('gbk')
        new_env['INNO_APP_VERSION'] = version.encode('gbk')
        tmp_output_dir = tmp_dir
        tmp_output_name = u'setup'
        if build_platform.lower() == 'win32':
          iss_file_path = u'bililivehime\\bililivehime.iss'
        else:
          iss_file_path = u'bililivehime\\bililivehime_x64.iss'
        args = [
            os.path.join(root_dir, u'InnoSetup\\iscc.exe'),
            #u'/qp',
            u'/o%s'%tmp_output_dir,
            u'/f%s'%tmp_output_name,
            os.path.join(root_dir, iss_file_path),
        ]
        p = subprocess.Popen(
            args, 
            shell = False, 
            cwd = root_dir, 
            env = new_env
        )
        p.wait()
        if os.path.exists( os.path.join(tmp_output_dir, tmp_output_name + '.exe') ):
            shutil.copyfile( 
                os.path.join(tmp_output_dir, tmp_output_name + '.exe'), 
                output_path
            )
        return p.returncode
    finally:
        remove_path(tmp_output_dir)
    return -1

def test():
    input_dir = os.path.join(root_dir, 'program_raw')
    version = '4.20.0.3405'
    temp_dir = os.path.join(root_dir, 'temp')
    output_file = os.path.join(root_dir, 'livehimeintl-release-x64-4.20.0.3405.exe')
    build_platform = 'x64'
    make(input_dir, version, temp_dir, output_file,build_platform)
    
if __name__ == '__main__':
    test()