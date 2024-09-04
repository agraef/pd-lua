
--[[
pdx.lua: useful extensions to pd.lua
Copyright (C) 2020 Albert Gräf <aggraef@gmail.com>

Currently there's only the pdx.reload() function, which implements a kind of
live coding functionality based on dofile and receivers, as explained in the
pd-lua tutorial. More may be added in the future.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--]]

local pdx = {}

--[[
Reload functionality. Call pdx.reload() on your object to enable, and
pdx.unreload() to disable this functionality again.

NOTE: As of pd-lua 0.12.8, this module is now pre-loaded, and pdx.reload()
gets called automatically before running the initialize method of any object,
so calling pdx.reload() explicitly is no longer needed. (Old code importing
the module and doing the call will continue to work, though.) Instead, you
will now have to call pdx.unreload() in your initialize method if you want to
*disable* this feature for some reason.

pdx.reload installs a "pdluax" receiver which reloads the object's script file
when it receives the "reload" message without any arguments, or a "reload
class" message with a matching class name.

Before reloading the script, we also execute the object's prereload() method if
it exists, and the postreload() method after reloading, so that the user can
customize object reinitialization as needed (e.g., to call initialize() after
reloading, or to perform some other custom reinitialization). This was
suggested by @ben-wes (thanks Ben!).

We have to go to some lengths here since we only want the script to be
reloaded *once* for each class, not for every object in the class. This
complicates things quite a bit. In particular, we need to keep track of object
deletions to perform the necessary bookkeeping, in order to ensure that at any
time there's exactly one participating object in the class which receives the
message. Which means that we also need to keep track of the object's finalize
method, so that we can chain to it in our own finalizer.
--]]

local reloadables = {}

-- finalize method, here we perform the necessary bookkeeping when an object
-- in the reloadables table gets deleted
local function finalize(self)
   if reloadables[self._name] then
      -- remove the object from the reloadables table, and restore its
      -- original finalizer
      pdx.unreload(self)
      -- call the object's own finalizer, if any
      if self.finalize then
	 self.finalize(self)
      end
   end
end

-- Our receiver. In the future, more functionality may be added here, but at
-- present this only recognizes the "reload" message and checks the class
-- name, if given.
local function pdluax(self, sel, atoms)
   if sel == "reload" and not string.match(self._scriptname, ".pd_luax$") then
      -- reload message, check that any extra argument matches the class name
      if atoms[1] == nil or atoms[1] == self._name then
         -- invoke the prereload method if it exists
         if self.prereload and type(self.prereload) == "function" then
            self:prereload()
         end
         pd.post(string.format("pdx: reloading %s", self._name))
         self:dofilex(self._scriptname)
         -- update the object's finalizer and restore our own, in case
         -- anything has changed there
         if self.finalize ~= finalize then
            reloadables[self._name][self].finalize = self.finalize
            self.finalize = finalize
         end
         -- invoke the postreload method if it exists
         if self.postreload and type(self.postreload) == "function" then
            local inlets, outlets = self.inlets, self.outlets
            self:postreload()
            -- recreate inlets and outlets as needed
            local function iolets_eq(a, b)
               if type(a) ~= type(b) then
                  return false
               elseif type(a) == "table" then
                  if #a ~= #b then
                     return false
                  else
                     for i = 1, #a do
                        if a[i] ~= b[i] then
                           return false
                        end
                     end
                     return true
                  end
               else
                  return a == b
               end
            end
            if not iolets_eq(self.inlets, inlets) then
               pd._createinlets(self._object, self.inlets)
            end
            if not iolets_eq(self.outlets, outlets) then
               pd._createoutlets(self._object, self.outlets)
            end
         end
      end
   end
end

-- purge an object from the reloadables table
function pdx.unreload(self)
   if reloadables[self._name] then
      if reloadables[self._name].current == self then
	 -- self is the current receiver, find another one
	 local current = nil
	 for obj, data in pairs(reloadables[self._name]) do
	    if type(obj) == "table" and obj ~= self then
	       -- install the receiver
	       data.recv = pd.Receive:new():register(obj, "pdluax", "_pdluax")
	       obj._pdluax = pdluax
	       -- record that we have a new receiver and bail out
	       current = obj
	       break
	    end
	 end
	 reloadables[self._name].current = current
	 -- get rid of the old receiver
	 reloadables[self._name][self].recv:destruct()
	 self._pdluax = nil
      end
      -- restore the object's finalize method
      self.finalize = reloadables[self._name][self].finalize
      -- purge the object from the reloadables table
      reloadables[self._name][self] = nil
      -- if the list of reloadables in this class is now empty, purge the
      -- entire class from the reloadables table
      if reloadables[self._name].current == nil then
	 reloadables[self._name] = nil
      end
   end
end

-- register a new object in the reloadables table
function pdx.reload(self)
   if reloadables[self._name] then
      -- We already have an object for this class, simply record the new one
      -- and install our finalizer so that we can perform the necessary
      -- cleanup when the object gets deleted. Also check that we don't
      -- register the same object twice (this won't really do any harm, but
      -- would be a waste of time).
      if reloadables[self._name][self] == nil then
	 reloadables[self._name][self] = { finalize = self.finalize }
	 self.finalize = finalize
      end
   else
      -- New class, make this the default receiver.
      reloadables[self._name] = { current = self }
      reloadables[self._name][self] = { finalize = self.finalize }
      -- install our finalizer
      self.finalize = finalize
      -- add the receiver
      reloadables[self._name][self].recv =
	 pd.Receive:new():register(self, "pdluax", "_pdluax")
      self._pdluax = pdluax
   end
end

return pdx
