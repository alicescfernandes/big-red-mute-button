from setuptools import setup
# https://py2app.readthedocs.io/en/latest/options.html
# python3 compile.py py2app
APP = ['mac_widget.py']
DATA_FILES = []
OPTIONS = {
    'argv_emulation': True,
    'plist': {
        'LSUIElement': True,
    },
    'packages': ['rumps','serial','Quartz'],
}

setup(
    app=APP,
    name="Big Red Mute Button",
    data_files=DATA_FILES,
    options={'py2app': OPTIONS},
    setup_requires=['py2app'],
)