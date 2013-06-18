/*****************************************************************************
 *   GATB : Genome Assembly Tool Box                                         *
 *   Authors: [R.Chikhi, G.Rizk, E.Drezen]                                   *
 *   Based on Minia, Authors: [R.Chikhi, G.Rizk], CeCILL license             *
 *   Copyright (c) INRIA, CeCILL license, 2013                               *
 *****************************************************************************/

#include <gatb/tools/misc/impl/Tool.hpp>
#include <gatb/system/impl/System.hpp>
#include <gatb/tools/misc/impl/Property.hpp>
#include <gatb/tools/misc/impl/Progress.hpp>
#include <gatb/tools/designpattern/impl/Command.hpp>

#define DEBUG(a)  //printf a

using namespace std;
using namespace gatb::core::system;
using namespace gatb::core::system::impl;

using namespace gatb::core::tools::dp;
using namespace gatb::core::tools::dp::impl;

/********************************************************************************/
namespace gatb {  namespace core { namespace tools {  namespace misc {  namespace impl {
/********************************************************************************/

const char* Tool::STR_NB_CORES      = "-nb-cores";
const char* Tool::STR_STATS_XML     = "-stats";
const char* Tool::STR_QUIET         = "-quiet";
const char* Tool::STR_NO_PROG_BAR   = "-no-progress-bar";
const char* Tool::STR_URI_PREFIX    = "-prefix";
const char* Tool::STR_URI_DATABASE  = "-db";
const char* Tool::STR_URI_OUTPUT    = "-out";
const char* Tool::STR_HELP          = "-help";

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
Tool::Tool (const std::string& name) : _name(name), _input(0), _output(0), _info(0), _parser(0), _dispatcher(0)
{
    setOutput (new Properties());

    setInfo  (new Properties());
    _info->add (0, _name);

    setParser (new OptionsParser ());
    _parser->add (new OptionOneParam (Tool::STR_NB_CORES,       "number of cores",                      false, "0"  ));
    _parser->add (new OptionOneParam (Tool::STR_STATS_XML,      "dump exec info into a XML file",       false       ));
    _parser->add (new OptionNoParam  (Tool::STR_QUIET,          "quiet execution",                      false       ));
    _parser->add (new OptionNoParam  (Tool::STR_NO_PROG_BAR,    "no progress bar",                      false       ));
    _parser->add (new OptionOneParam (Tool::STR_URI_PREFIX,     "prefix to be appended to temp files",  false, ""   ));
    _parser->add (new OptionOneParam (Tool::STR_URI_DATABASE,   "databank uri",                         false, ""   ));
    _parser->add (new OptionOneParam (Tool::STR_URI_OUTPUT,     "output",                               false, ""   ));
    _parser->add (new OptionNoParam  (Tool::STR_HELP,           "display help about possible options",  false       ));
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
Tool::~Tool ()
{
    setInput      (0);
    setOutput     (0);
    setInfo       (0);
    setParser     (0);
    setDispatcher (0);
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
IProperties* Tool::run (int argc, char* argv[])
{
    return run (getOptionsParser()->parse (argc, argv));
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
IProperties* Tool::run (IProperties* input)
{
    /** We keep the input parameters. */
    setInput (input);

    /** We define one dispatcher. */
    setDispatcher (new ParallelCommandDispatcher (_input->getInt(STR_NB_CORES)) );

    /** We may have some pre processing. */
    preExecute ();

    /** We execute the actual job. */
    {
        //TIME_INFO (_timeInfo, _name);
        execute ();
    }

    /** We may have some post processing. */
    postExecute ();

    /** We return the output properties. */
    return _output;
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Tool::preExecute ()
{
    /** We add a potential config file to the input properties. */
    _input->add (1, new Properties (System::info().getHomeDirectory() + "/." + getName() ));

    /** We may have to add a default prefix for temporary files. */
    if (_input->get(STR_URI_PREFIX)==0)  { _input->add (1, STR_URI_PREFIX, "tmp.");  }

    /** We may have to add a default prefix for temporary files. */
    if (_input->getInt(STR_NB_CORES)<=0)  { _input->setInt (STR_NB_CORES, System::info().getNbCores());  }

    /** We add the input properties to the statistics result. */
    _info->add (1, _input);
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Tool::postExecute ()
{
    /** We add the time properties to the output result. */
    _info->add (1, _timeInfo.getProperties ("time"));

    /** We add the output properties to the output result. */
    _info->add (1, "output");
    _info->add (2, _output);

    /** We may have to dump execution information into a stats file. */
    if (_input->get(Tool::STR_STATS_XML) != 0)
    {
        XmlDumpPropertiesVisitor visit (_info->getStr (Tool::STR_STATS_XML));
        _info->accept (&visit);
    }

    /** We may have to dump execution information to stdout. */
    if (_input->get(Tool::STR_QUIET) == 0)
    {
        RawDumpPropertiesVisitor visit;
        _info->accept (&visit);
    }
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
dp::IteratorListener* Tool::createIteratorListener (size_t nbIterations, const char* message)
{
    return new Progress (nbIterations, message);
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
ToolComposite::ToolComposite (const std::string& name) : Tool(name)
{
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
ToolComposite::~ToolComposite ()
{
    for (list<Tool*>::iterator it = _tools.begin(); it != _tools.end(); it++)
    {
        (*it)->forget ();
    }
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
IProperties* ToolComposite::run (int argc, char* argv[])
{
    IProperties* output = 0;

    for (list<Tool*>::iterator it = _tools.begin(); it != _tools.end(); it++)
    {
        /** We get the parameters from the current parser. */
        IProperties* input = (*it)->getOptionsParser()->parse (argc, argv);

        /** We may have to add the output of the previous tool to the input of the current tool.
         *  WARNING! The output of the previous tool should have a bigger priority than the
         *  user parameters of the current tool.
         */
        IProperties* actualInput = 0;
        if (output != 0)
        {
            actualInput = new Properties();
            actualInput->add (1, output);   // output of the previous tool
            actualInput->add (1, input);    // input  of the previous tool
        }
        else
        {
            actualInput = input;
        }

        /** We run the tool and get a reference on its output. */
        output = (*it)->run (actualInput);

        /** We add the current tool info to the global properties. */
        _info->add (1, (*it)->getInfo());
    }

    /** We return the output properties. */
    return _output;
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void ToolComposite::add (Tool* tool)
{
    if (tool)
    {
        tool->use ();
        _tools.push_back(tool);
    }
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void ToolComposite::execute ()
{
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void ToolComposite::preExecute ()
{
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void ToolComposite::postExecute ()
{
    /** We may have to dump execution information into a stats file. */
    if (_input->get(Tool::STR_STATS_XML) != 0)
    {
        XmlDumpPropertiesVisitor visit (_info->getStr (Tool::STR_STATS_XML), false);
        _info->accept (&visit);
    }
}

/********************************************************************************/
} } } } } /* end of namespaces. */
/********************************************************************************/
