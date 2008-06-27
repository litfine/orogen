#include <rtt/os/main.h>
<% if component.toolkit %>#include "toolkit/<%= toolkit.name %>Toolkit.hpp"<% end %>
<% component.used_toolkits.each do |name| %>
#include <toolkit/<%= name %>Toolkit.hpp>
<% end %>
<% component.tasks.each do |task|
    next if task.external_definition?  %>
#include <tasks/<%= task.name %>.hpp>
<% end %>
<% if deployer.corba_enabled? %>
#include <rtt/corba/ControlTaskServer.hpp>
using RTT::Corba::ControlTaskServer;
<% end %>

#include <rtt/PeriodicActivity.hpp>
#include <rtt/NonPeriodicActivity.hpp>

<% if deployer.browse %>
#include <ocl/TaskBrowser.hpp>
<% end %>

using namespace Orocos;
int ORO_main(int argc, char* argv[])
{
   <% if deployer.loglevel %>
   if ( log().getLogLevel() < Logger::<%= deployer.loglevel %> ) {
       log().setLogLevel( Logger::<%= deployer.loglevel %> );
   }
   <% end %>

   <% if component.toolkit %>RTT::Toolkit::Import( <%= component.toolkit.name %>::Toolkit );<% end %>
   <% component.used_toolkits.each do |name| %>
   RTT::Toolkit::Import( <%= name %>::Toolkit );
   <% end %>

<% if deployer.corba_enabled? %>
    ControlTaskServer::InitOrb(argc, argv);
<% end %>

    TaskContext* main_task = 0;
<% deployer.task_activities.each do |task| %>
    <%= component.name %>::<%= task.context.name %> task_<%= task.name%>("<%= task.name %>");
    <% if task.period %>RTT::PeriodicActivity<% else %>RTT::NonPeriodicActivity<% end %>
    activity_<%= task.name%>(
            <%= task.rtt_scheduler %>,
            <%= task.rtt_priority %>,
            <% if task.period %><%= task.period %>, <% end %>
            task_<%= task.name%>.engine());
    <% if deployer.corba_enabled? %>
    ControlTaskServer::Create( &task_<%= task.name%> );
    <% end %>
<% end %>

    // Start some activities
<% deployer.task_activities.each do |task|
    if task.start?
        if task.context.initial_state == 'PreOperational' %>
    if (!task_<%= task.name %>.configure())
    {
        log(Error) << "cannot configure <%= task.name %>" << endlog();
        return -1;
    }
        <% end %>
    if (!task_<%= task.name %>.start())
    {
        log(Error) << "cannot start <%= task.name %>" << endlog();
        return -1;
    }
    <% end %>
<% end %>

<% if deployer.corba_enabled? %>
    ControlTaskServer::RunOrb();
    ControlTaskServer::DestroyOrb();
<% elsif deployer.browse %>
    TaskBrowser browser(& task_<%= deployer.browse.name %>);
    browser.loop();
<% end %>

    return 0;
}

