def toTex(basename):
    print('Converting '+basename+'.md')
    if subprocess.call(['pandoc','-S','--listings','-f','markdown_github','-V geometry:margin=.5in','-o',basename+'.tex',basename+'.md']):
        print('Failue pandoc')
import os
import glob
import subprocess
import fileinput
for name in glob.glob('Retro-Graphics-Toolkit.wiki/*.md'):
    basename=extension = os.path.splitext(name)[0]
    texname=basename
    texname+='.tex'
    if os.path.isfile(texname):
        # Check if md file is newer than tex file
        if os.stat(name).st_mtime>os.stat(texname).st_mtime:
            toTex(basename)
    else:
        toTex(basename)
if not os.path.isfile('Retro-Graphics-Toolkit.wiki/Home.md'):
    print('Need a home file')
    quit()
concat=""
concat += open('header.tex').read()
concat+='\chapter{Introduction}\n'
concat += open('Home-offline.tex').read()
for name in glob.glob('Retro-Graphics-Toolkit.wiki/*.tex'):
    if 'Retro-Graphics-Toolkit.wiki/Home.tex'!=name:
        basename=os.path.basename(os.path.splitext(name)[0])
        basename=basename.replace('-',' ')
        concat+='\chapter{'+basename+'}\n'
        concat += open(name).read()
concat += open('footer.tex').read()
concat=concat.replace('includegraphics{https://raw.githubusercontent.com/ComputerNerd/Retro-Graphics-Toolkit/master/','includegraphics{../')
with open("Manual.tex",'w') as f:
    f.write(concat)
args=['xelatex','Manual']
if subprocess.call(args):
    print('xelatex faill pass 1')
if subprocess.call(args):
    print('xelatex faill pass 2')
