__author__ = 'Robert Ancell <bob27@users.sourceforge.net>'
__license__ = 'GNU General Public License Version 2'
__copyright__ = 'Copyright 2005-2006  Robert Ancell'

import os
import sys
import select
import xml.dom.minidom

import game
import cecp
import uci

from defaults import *

CECP = 'CECP'
UCI  = 'UCI'

class Option:
    value = ''
    
    pass

class Profile:
    """
    """
    name       = ''
    protocol   = ''
    executable = ''
    path       = ''
    arguments  = None
    options    = None
    
    def __init__(self):
        self.arguments = []
        self.options = []

    def detect(self):
        """
        """
        try:
            path = os.environ['PATH'].split(os.pathsep)
        except KeyError:
            path = []

        for directory in path:
            b = directory + os.sep + self.executable
            if os.path.isfile(b):
                self.path = b
                return
        self.path = None
        
def _getXMLText(node):
    """
    """
    if len(node.childNodes) == 0:
        return ''
    if len(node.childNodes) > 1 or node.childNodes[0].nodeType != node.TEXT_NODE:
        raise ValueError
    return node.childNodes[0].nodeValue 

def loadProfiles():
    """
    """
    profiles = []
    
    fileNames = [os.path.expanduser('~/.glchess/ai.xml'), os.path.join(BASE_DIR,'ai.xml'), 'ai.xml']
    document = None
    for f in fileNames:
        try:
            document = xml.dom.minidom.parse(f)
        except IOError:
            pass
        except xml.parsers.expat.ExpatError:
            print 'AI configuration at ' + f + ' is invalid, ignoring'
        else:
            print 'Loading AI configuration from ' + f
            break
    if document is None:
        print 'WARNING: No AI configuration'
        return profiles

    elements = document.getElementsByTagName('aiconfig')
    if len(elements) == 0:
        return profiles

    for e in elements:
        for p in e.getElementsByTagName('ai'):
            try:
                protocolName = p.attributes['type'].nodeValue
            except KeyError:
                assert(False)
            if protocolName == 'cecp':
                protocol = CECP
            elif protocolName == 'uci':
                protocol = UCI
            else:
                assert(False), 'Uknown AI type: ' + repr(protocolName)
            
            n = p.getElementsByTagName('name')
            assert(len(n) > 0)
            name = _getXMLText(n[0])
            
            n = p.getElementsByTagName('binary')
            assert(len(n) > 0)
            executable = _getXMLText(n[0])
            
            arguments = [executable]
            n = p.getElementsByTagName('argument')
            for x in n:
                args.append(_getXMLText(x))
            
            options = []
            n = p.getElementsByTagName('option')
            for x in n:
                option = Option()
                option.value = _getXMLText(x)
                try:
                    attribute = x.attributes['name']
                except KeyError:
                    pass
                else:
                    option.name = _getXMLText(attribute)
                options.append(option)

            profile = Profile()
            profile.name       = name
            profile.protocol   = protocol
            profile.executable = executable
            profile.arguments  = arguments
            profile.options    = options
            profiles.append(profile)
    
    return profiles

class CECPConnection(cecp.Connection):
    """
    """
    player = None
    
    def __init__(self, player):
        """
        """
        self.player = player
        cecp.Connection.__init__(self)
        
    def onOutgoingData(self, data):
        """Called by cecp.Connection"""
        self.player.logText(data, 'output')
        self.player.sendToEngine(data)

    def onMove(self, move):
        """Called by cecp.Connection"""
        self.player.moving = True
        self.player.move(move)
        
    def logText(self, text, style):
        """Called by cecp.Connection"""
        self.player.logText(text, style)

class UCIConnection(uci.StateMachine):
    """
    """
    player = None
    
    def __init__(self, player):
        """
        """
        self.player = player
        uci.StateMachine.__init__(self)
        
    def onOutgoingData(self, data):
        """Called by uci.StateMachine"""
        self.player.logText(data, 'output')
        self.player.sendToEngine(data)
        
    def logText(self, text, style):
        """Called by uci.StateMachine"""
        self.player.logText(text, style)

    def onMove(self, move):
        """Called by uci.StateMachine"""
        self.player.move(move)

class Player(game.ChessPlayer):
    """
    """
    
    # The profile we are using
    __profile = None
    
    __pipeFromEngine = None
    __pipeToEngine = None
    
    # PID of the subprocess containing the engine
    __pid = None
    
    __connection = None
    
    moving = False
    
    def __init__(self, name, profile):
        """Constructor for an AI player.
        
        'name' is the name of the player (string).
        'profile' is the profile to use for the AI (Profile).
        """
        self.__profile = profile

        game.ChessPlayer.__init__(self, name)
        
        # Create pipes for communication with AI process
        self.__pipeToEngine = os.pipe()
        self.__pipeFromEngine = os.pipe()
        
        # Fork sub-process for engine
        self.__pid = os.fork()

        # This is the forked process, replace it with the engine
        if self.__pid == 0:
            self.__startEngine(profile.path, profile.arguments)

        # Close the ends of the pipe we are not using
        os.close(self.__pipeFromEngine[1]);
        os.close(self.__pipeToEngine[0]);
        
        if profile.protocol == CECP:
            self.connection = CECPConnection(self)
        elif profile.protocol == UCI:
            self.connection = UCIConnection(self)
        else:
            assert(False)
            
        self.connection.start()
        self.connection.configure(profile.options)

    # Methods to extend
    
    def logText(self, text, style):
        """
        """
        pass
        
    # Public methods
    
    def getProfile(self):
        """
        """
        return self.__profile
     
    def fileno(self):
        """Returns the file descriptor for communicating with the engine (integer)"""
        return self.__pipeFromEngine[0]

    def read(self):
        """Read an process data from the engine.
        
        This is non-blocking.
        """
        while True:
            # Check if data is available
            (rlist, _, _) = select.select([self.__pipeFromEngine[0]], [], [], 0)
            if len(rlist) == 0:
                return
            
            # Read a chunk and process
            data = os.read(self.__pipeFromEngine[0], 256)
            if data == '':
                return
            self.connection.registerIncomingData(data)
            
    def sendToEngine(self, data):
        """
        """
        try:
            os.write(self.__pipeToEngine[1], data)
        except OSError, e:
            print 'Failed to write to engine: ' + str(e)

    def quit(self):
        """Disconnect the AI"""
        # Wait for the pipe to close
        # There must be a better way of doing this!
        count = 0
        while True:
            select.select([], [], [], 0.1)
            try:
                os.write(self.__pipeToEngine[1], '\nquit\n') # FIXME: CECP specific
            except OSError:
                return
            count += 1
            if count > 5:
                break
        
        print 'Killing AI ' + str(self.__pid)
        os.kill(self.__pid, 9)

    # Extended methods
    
    def onPlayerMoved(self, player, move):
        """Called by game.ChessPlayer"""
        isSelf = player is self and self.moving
        self.moving = False
        self.connection.reportMove(move.canMove, isSelf)
    
    def readyToMove(self):
        """Called by game.ChessPlayer"""
        self.connection.requestMove()
        
    def onGameEnded(self, winningPlayer = None):
        """Called by game.ChessPlayer"""
        self.quit()

    # Private methods
    
    def __startEngine(self, executable, arguments):
        """
        """
        # Connect stdin and stdout to the pipes to the main process
        os.dup2(self.__pipeFromEngine[1], sys.stdout.fileno())
        os.dup2(self.__pipeToEngine[0], sys.stdin.fileno())
        
        # Close the ends of the pipe we are not using
        os.close(self.__pipeFromEngine[0]);
        os.close(self.__pipeToEngine[1]);
        
        # Make the process nice so it doesn't hog the CPU
        os.nice(15)

        # Start the AI
        try:
            os.execv(executable, arguments)
        except OSError:
            select.select([], [], [], None)
            os._exit(os.EX_UNAVAILABLE)