def toTex(basename):
    print('Converting to latex '+basename+'.md')
    if subprocess.call(['pandoc','-S','--listings','-f','markdown','-V geometry:margin=.5in','-o',basename+'.tex',basename+'.md']):
        print('pandoc failure')
def toGithub(inmd,outmd):
    print('Converting to github markdown '+inmd)
    if subprocess.call(['pandoc','-S','-f','markdown','-t','markdown_github','-o',outmd,inmd]):
        print('pandoc failure')
import os
import glob
import subprocess
import fileinput
import sys
for name in glob.glob('Pandoc/*.md'):
    githubPath='Retro-Graphics-Toolkit.wiki/'+os.path.basename(name)
    if os.path.isfile(githubPath):
        if os.stat(name).st_mtime>os.stat(githubPath).st_mtime:
            toGithub(name,githubPath)
    else:
        toGithub(name,githubPath)
if len(sys.argv)<=1:
    for name in glob.glob('Pandoc/*.md'):
        basename=extension = os.path.splitext(name)[0]
        texname=basename
        texname+='.tex'
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
    for name in glob.glob('Pandoc/*.tex'):
        if 'Pandoc/Home.tex'!=name:
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
else:
    print('Skipping offline manual generation')
