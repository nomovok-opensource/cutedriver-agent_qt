# Project-specific configuration for CruiseControl.rb

Project.configure do |project|
 
  project.email_notifier.emails = ['ext-mika.1.leiman@nokia.com']
  
  project.email_notifier.from = 'ext-mika.1.leiman@nokia.com'

  # Build the project by invoking rake task 'custom'
  # project.rake_task = 'custom'

  # Build the project by invoking shell script "build_my_app.sh". Keep in mind that when the script is invoked,
  # current working directory is <em>[cruise&nbsp;data]</em>/projects/your_project/work, so if you do not keep build_my_app.sh
  # in version control, it should be '../build_my_app.sh' instead
  # project.build_command = 'build_my_app.sh'

  # Ping Subversion for new revisions every 5 minutes (default: 30 seconds)
  # project.scheduler.polling_interval = 5.minutes

end
