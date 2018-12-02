#!/usr/bin/python3
def toTex(basename):
    print('Converting to latex '+basename+'.md')
    if subprocess.call(['pandoc','--listings','-f','markdown', '-t', 'latex+smart', '-V', 'geometry:margin=.5in','-o',basename+'.tex',basename+'.md']):
        print('pandoc failure')
def toGithub(inmd,outmd):
    print('Converting to github markdown '+inmd)
    if subprocess.call(['pandoc','-f','markdown','-t','gfm+smart','-o',outmd,inmd]):
        print('pandoc failure')
    f = open(outmd,'r+')
    txt=f.read()
    txt=txt.replace('\\[\\[','[[')
    txt=txt.replace('\\]\\]',']]')
    f.seek(0)
    f.write(txt)
    f.truncate()
    f.close()
import os
import glob
import subprocess
import fileinput
import sys
import re
for name in sorted(glob.glob('Pandoc/*.md')):
    githubPath='Retro-Graphics-Toolkit.wiki/'+os.path.basename(name)
    if os.path.isfile(githubPath):
        if os.stat(name).st_mtime>os.stat(githubPath).st_mtime:
            toGithub(name,githubPath)
    else:
        toGithub(name,githubPath)
if len(sys.argv)<=1:
    for name in sorted(glob.glob('Pandoc/*.md')):
        basename = os.path.splitext(name)[0]
        texname = basename + '.tex'
        if os.path.isfile(texname):
            # Check if md file is newer than tex file
            if os.stat(name).st_mtime>os.stat(texname).st_mtime:
                toTex(basename)
        else:
            toTex(basename)
    concat=""
    concat += open('header.tex').read()
    concat+='\chapter{Introduction}\n'
    concat += open('Home-offline.tex').read()
    concat+='\mainmatter\n'
    for name in sorted(glob.glob('Pandoc/*.tex')):
        if 'Pandoc/Home.tex'!=name:
            basename=os.path.basename(os.path.splitext(name)[0])
            concat+='\\chapter{'+basename.replace('-',' ')+'}\n'
            concat+='\\label{'+basename+'}\n'
            concat += open(name).read()
    concat += open('footer.tex').read()
    concat=concat.replace('includegraphics{https://raw.githubusercontent.com/ComputerNerd/Retro-Graphics-Toolkit/master/','includegraphics{../')
    while True:
        fs=concat.find('{[}{[}')
        fm=concat.find('\\textbar{}')
        fe=concat.find('{]}{]}')
        if fs<0:
            break
        labelText=concat[fs+6:fm]
        labelName=concat[fm+10:fe]
        before=concat[0:fs]
        after=concat[fe+6:]
        print(labelText,labelName)
        concat=before+'\\hyperref['+labelName+']{'+labelText+'}'+after
    with open("Manual.tex",'w') as f:
        f.write(concat)
    args=['xelatex','Manual']
    if subprocess.call(args):
        print('xelatex fail pass 1')
    if subprocess.call(args):
        print('xelatex fail pass 2')
else:
    print('Skipping offline manual generation')
