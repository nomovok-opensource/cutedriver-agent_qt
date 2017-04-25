############################################################################
## 
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
## All rights reserved. 
## Contact: Nokia Corporation (testabilitydriver@nokia.com) 
## 
## This file is part of Testability Driver Qt Agent
## 
## If you have questions regarding the use of this file, please contact 
## Nokia at testabilitydriver@nokia.com . 
## 
## This library is free software; you can redistribute it and/or 
## modify it under the terms of the GNU Lesser General Public 
## License version 2.1 as published by the Free Software Foundation 
## and appearing in the file LICENSE.LGPL included in the packaging 
## of this file. 
## 
############################################################################





@__release_mode = ENV['rel_mode']
@__release_mode = 'minor' if @__release_mode == nil
  
# version information
def read_version
	version = "0"
	File.open(Dir.getwd << '/debian/changelog') do |file|
		line = file.gets
		arr = line.split(')')
		arr = arr[0].split('(')
		version = arr[1]
	end
	
	if(@__release_mode == 'release')
		return version
	else
		return version + "~" + Time.now.strftime("%Y%m%d%H%M%S")   
	end
end

@__tas_revision = read_version
puts "version " << @__tas_revision

desc "Task for cruise control"
task :cruise => ['kill_qttasserver', 'build_qttas',  'start_qttasserver'] do
	exit(0)
end

desc "Task for cruise control"
task :cruise_vs => ['kill_qttasserver', 'build_qttas_vs',  'start_qttasserver'] do
	exit(0)
end

desc "Task for building the example QT application(s)"
task :build_qttas do

  puts "#########################################################"
  puts "### Building qttas                                   ####"
  puts "#########################################################"

	# buid version file
	#File.open('common/inc/version.h', 'w') { |f| f.write "static QString TAS_VERSION = \"#{@__tas_revision}\";" }  
  make = "make"
  sudo = ""	

  if /win/ =~ RUBY_PLATFORM || /mingw32/ =~ RUBY_PLATFORM
    make = "mingw32-make"
  else
    sudo = "echo \"testability\" | sudo -S "
  end
  cmd = sudo + " #{make} uninstall"
  system(cmd)

  cmd = "#{make} distclean"
  system(cmd)
  
  cmd = "qmake CONFIG+=release qt_testability.pro"
  failure = system(cmd)
  raise "qmake failed" if (failure != true) or ($? != 0) 
    
  cmd = "#{make}"
  failure = system(cmd)
  raise "make release failed" if (failure != true) or ($? != 0) 
    
  cmd = sudo + "#{make} install"
  failure = system(cmd)
  raise "make install failed" if (failure != true) or ($? != 0) 
  puts "qttas built"
end

desc "Task for building the example QT application(s)"
task :build_qttas_vs do

  puts "#########################################################"
  puts "### Building qttas                                   ####"
  puts "#########################################################"

	# buid version file
	#File.open('common/inc/version.h', 'w') { |f| f.write "static QString TAS_VERSION = \"#{@__tas_revision}\";" }  
  make = "nmake"
  sudo = ""	

  cmd = sudo + " #{make} uninstall"
  system(cmd)

  cmd = "#{make} distclean"
  system(cmd)
  
  cmd = "qmake CONFIG+=release qt_testability.pro"
  failure = system(cmd)
  raise "qmake failed" if (failure != true) or ($? != 0) 
    
  cmd = "#{make}"
  failure = system(cmd)
  raise "make release failed" if (failure != true) or ($? != 0) 
    
  cmd = sudo + "#{make} install"
  failure = system(cmd)
  raise "make install failed" if (failure != true) or ($? != 0) 
  puts "qttas built"
end


desc "Task for shutting down the qttasserver"
task :kill_qttasserver do

  puts "#########################################################"
  puts "### Killing qttasserver                              ####"
  puts "#########################################################"

  if /win/ =~ RUBY_PLATFORM || /mingw32/ =~ RUBY_PLATFORM

    cmd = "taskkill /F /IM qttasserver.exe"
    stdOut = system(cmd)
  else
    cmd = "echo \"testability\" | sudo -S killall qttasserver"
    stdOut = system(cmd)
  end
	puts "qttasserver killed"
end

desc "Task for starting the qttasserver"
task :start_qttasserver do

  puts "#########################################################"
  puts "### Starting qttasserver                             ####"
  puts "#########################################################"

  if /win/ =~ RUBY_PLATFORM || /mingw32/ =~ RUBY_PLATFORM
  	require 'win32ole'
		begin
      shell = WIN32OLE.new( 'Shell.Application' )
      shell.ShellExecute( 'qttasserver.exe', '', 'f:\qttas\bin', '', 2)      
    rescue
	  raise "Qttasserver startup failed"		
	end
  else
    # directly calling "/usr/bin/qttasserver &" hangs, now disabled the stdin, out and err    
    result=`qttasserver </dev/null >/dev/null 2>&1 &`		
  end
  
  puts "qttasserver started"
end


