import os
import pprint
import logging

import jinja2
    
LOADER = jinja2.FileSystemLoader(os.path.realpath(os.path.dirname(__file__)))
ENV    = jinja2.Environment(loader=LOADER, trim_blocks=True, lstrip_blocks=True, undefined=jinja2.StrictUndefined)

def renderAsString(templateName, context):
    """Return a template rendered as a String using the given context"""
    template = ENV.get_template(templateName)
    
    try:
        return template.render(context)
    except Exception:
        pp = pprint.PrettyPrinter(depth=8)
        logging.getLogger("").error("could not render template '%s'" % templateName)
        logging.getLogger("").error("used context:")
        logging.getLogger("").error(pp.pformat(context))
        raise

def render(templateName, outputFilePath, context):
    """Render a template to outputFilename using the given context.
    Always overwrite.
    """
    absolutePath  = outputFilePath
    canonicalPath = os.path.realpath(absolutePath)
    
    with open(canonicalPath, "w") as output:
        output.write(renderAsString(templateName, context))
    return canonicalPath, context

class HipImplementationModel():
    def generateCode(self,outputFilePath,context):
        return render("templates/HipImplementation.cpp-template", outputFilePath, context)

class InterfaceModuleModel():
    def generateCode(self,outputFilePath,context):
        return render("templates/InterfaceModule.f03-template", outputFilePath, context)

class InterfaceModuleTestModel():
    def generateCode(self,outputFilePath,context):
        return render("templates/InterfaceModuleTest.f03-template", outputFilePath, context) 
