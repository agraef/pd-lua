local lua_sig = pd.Class:new():register("lua_sig")

function lua_sig:initialize(sel, atoms)
    self.inlets = {SIGNAL, DATA, DATA}
    self.outlets = {SIGNAL}
    self.phase = 0
    self.freq = 220
    self.amp = 0.5
    return true
end

function lua_sig:in_2_float(freq)
    self.freq = freq
end

function lua_sig:in_3_float(amp)
    self.amp = amp
end

-- dsp method is called when processing is about to start
function lua_sig:dsp(samplerate, blocksize)
    self.samplerate = samplerate
end

-- perform method gets called with a table for each signal inlet
-- you must return a table for each signal outlet
function lua_sig:perform(in1)
    local frequency = self.freq
    local amplitude = self.amp

    local angular_freq = 2 * math.pi * frequency / self.samplerate

    -- Loop through each sample index
    for i = 1, #in1 do
        in1[i] = amplitude * math.sin(self.phase)
        self.phase = self.phase + angular_freq
        if self.phase >= 2 * math.pi then
            self.phase = self.phase - 2 * math.pi
        end
    end

    return in1
end
