import mido

NOTE_val2str = {}
for i in range(21,127+1):
    # notename
    if (i-21)%12==0:  notename='LA'
    if (i-21)%12==1:  notename='SIb'
    if (i-21)%12==2:  notename='SI'
    if (i-21)%12==3:  notename='DO'
    if (i-21)%12==4:  notename='REb'
    if (i-21)%12==5:  notename='RE'
    if (i-21)%12==6:  notename='MIb'
    if (i-21)%12==7:  notename='MI'
    if (i-21)%12==8:  notename='FA'
    if (i-21)%12==9:  notename='SOLb'
    if (i-21)%12==10: notename='SOL'
    if (i-21)%12==11: notename='LAb'
    # octave
    octave = int((i-12)/12)
    # name
    NOTE_val2str[i] = f'NOTE_{notename}_{octave}'

class MidiParser(object):
    
    MAXNUMNOTES     = 1000
    PRIORITY_TRACKS = [
        'Bassoon',
        'Flute',
        'Horns',
        'Trumpets',
        'Trombones',
        'Harp',
        'Piano',
        'Strings',
        'String 2',
        'String 3',
    ]
    
    def __init__(self,filename,songname,maxduration,numtracks=10):
        
        # initialize
        self.filename        = filename
        self.songname        = songname
        self.maxduration     = maxduration
        self.numtracks       = numtracks
        self.mid             = mido.MidiFile(self.filename)
        
        # print summary
        print(self._format_summary())
        
        # extract
        self.extract    = {}
        for trackname in self.PRIORITY_TRACKS:
            self.extract[trackname] = self._extract_track(trackname)
        
        # write
        self._write_extract(self.songname,self.extract)
    
    #======================== public ==========================================
    
    #======================== private =========================================
    
    def _format_summary(self):
        output  = []
        output += ['']
        output += ['Extracting {} tracks from "{}"'.format(self.numtracks,self.filename)]
        output += ['']
        output += ['Initial file contains {} tracks:'.format(len(self.mid.tracks))]
        for (i,track) in enumerate(self.mid.tracks):
            output += ['   {:>2}. {}'.format(i,track.name)]
        output += ['']
        output  = '\n'.join(output)
        return output
    
    def _extract_track(self,trackname):
        
        print(f'extracting track "{trackname}"...',end='')
        
        input_track = self._find_track(trackname)
        
        returnVal   = []
        cur_note    = None
        ts          = 0
        start_ts    = 0
        for msg in input_track:
            
            ts += msg.time
            
            if msg.type=='note_on' and msg.velocity>0 and cur_note==None:
                # note starts
                
                # log how long there has been a silence
                duration = ts-start_ts
                if duration>1:
                    returnVal  += [(None,duration)]
                
                # remember the current note and when it started
                cur_note = msg.note
                start_ts = ts
            
            if msg.type=='note_on' and msg.velocity==0 and msg.note==cur_note:
                # note ends
                
                # log how long the last note has played
                duration = ts-start_ts
                if duration>1:
                    returnVal  += [(NOTE_val2str[cur_note],duration)]
                
                # remember this is a silence and when it started
                cur_note = None
                start_ts = ts
            
            if ts>self.maxduration:
                break
        
        print(f'   done ({len(returnVal)} notes)')
        return returnVal
    
    def _find_track(self,trackname):
        for track in self.mid.tracks:
            if track.name==trackname:
                return track
        raise ValueError()
    
    def _write_extract(self,songname,extract):
        output   = []
        output  += [f'#ifndef __{songname}_H']
        output  += [f'#define __{songname}_H']
        output  += ['']
        trackidx = 0
        for (trackname,tracknotes) in extract.items():
            output += [self._format_track(songname,trackname,trackidx,tracknotes)]
            trackidx += 1
            output  += ['']
        output  += ['#endif']
        output  += ['']
        output  = '\n'.join(output)
        filename = f'song_{songname}.h'
        with open(filename,'w') as f:
            f.write(output)
        print(f'\nWritten to {filename}')
    
    def _format_track(self,songname,trackname,trackidx,tracknotes):
        output = []
        output += [f'static const note_t SONGNOTES_{songname}_TRACK_{trackidx}[] = {{ // {trackname}']
        for (note,dur) in tracknotes[:self.MAXNUMNOTES]:
            if note is None:
                note = 'NOTE_NONE'
            output += ['    {{{:<12} {}}},'.format(note+',',dur)]
        output += ['};']
        output  = '\n'.join(output)
        return output
    
#============================ main ============================================

def main():
    MidiParser(
        filename    = 'Star_Wars_Medley.mid',
        songname    = 'STAR_WARS',
        maxduration = 6600,
    )

if __name__=='__main__':
    main()