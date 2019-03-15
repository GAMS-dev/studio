/*
 * This file is part of the GAMS Studio project.
 *
 * Generated by C:\home\Lutz\vs8_alpha\src\gamscmex\gmsdco.gms on 07/27/18 11:45:57
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYNTAXDATA_H
#define SYNTAXDATA_H

#include <QList>
#include <QString>
#include <QPair>

namespace gams {
namespace studio {
namespace syntax {

class SyntaxData
{
    SyntaxData() {}
public:
    static QList<QPair<QString, QString>> directives() {
        QList<QPair<QString, QString>> list = {
            {"onExternalInput", "External input on"},
            {"offExternalInput", "External input off"},
            {"onExternalOutput", "External output on"},
            {"offExternalOutput", "External output off"},
            {"killUel", "Remove UEL List"},
            {"onMargin", "Margin marking on"},
            {"offMargin", "Margin marking off"},
            {"minCol", "Set left data margin"},
            {"maxCol", "Set right data margin"},
            {"comment", "Reset comment symbol"},
            {"dollar", "Reset dollar symbol"},
            {"onDigit", "Do not allow excess precision in constants"},
            {"offDigit", "Ignore excess precision in constants"},
            {"hidden", "Hidden comment line"},
            {"onText", "Start of block comment"},
            {"offText", "End of block comment"},
            {"onUpper", "Uppercase listing file on"},
            {"offUpper", "Uppercase listing file off"},
            {"single", "Single space input in listing file"},
            {"double", "Double space input in listing file"},
            {"lines", "Eject listing if less than n lines"},
            {"title", "Set listing page title"},
            {"sTitle", "Set listing page subtitle"},
            {"onSymList", "Produce symbol list"},
            {"offSymList", "Do not produce symbol list"},
            {"onUelList", "Produce UEL list"},
            {"offUelList", "Do not produce UEL list"},
            {"onSymXRef", "Collect symbols for cross reference list"},
            {"offSymXRef", "Do not collect symbols for cross reference list"},
            {"onUelXRef", "Collect UELs for cross reference list"},
            {"offUelXRef", "Do not collect UELs for cross reference list"},
            {"debug", "Debugging options"},
            {"onDollar", "Show dollar control lines on"},
            {"offDollar", "Show dollar control lines off"},
            {"phantom", "Define phantom element"},
            {"version", "Test GAMS compiler version number"},
            {"call", "Execute another program"},
            {"callAsync", "Execute another program async (inherit console)"},
            {"callAsyncIC", "Execute another program async (inherit console)"},
            {"callAsyncNC", "Execute another program async (new console)"},
            {"hiddenCall", "Execute another program (hidden)"},
            {"include", "Include file from working directory"},
            {"onInclude", "Include message on"},
            {"offInclude", "Include message off"},
            {"sysInclude", "Include file from system directory"},
            {"libInclude", "Include file from library directory"},
            {"insert", "Insert external procedure information"},
            {"sysInsert", "Insert external procedure information from system directory"},
            {"libInsert", "Insert external procedure information from library directory"},
            {"batInclude", "Include file with substitution arguments"},
            {"onCmsIncl", "CMS batinclude format on"},
            {"offCmsIncl", "CMS batinclude format off"},
            {"funcLibIn", "Load extrinsic function library"},
            {"goto", "Goto a label"},
            {"label", "Label definition"},
            {"maxGoTo", "Maximum number of jumps to the same label"},
            {"if", "If statement case sensitive"},
            {"ifI", "If statement case insensitive"},
            {"ifE", "If statement with expression evaluation"},
            {"shift", "Shift input arguments"},
            {"onNestCom", "Nested comments on"},
            {"offNestCom", "Nested comments off"},
            {"onInline", "Enable inline comments"},
            {"offInline", "Disable inline comments"},
            {"inlineCom", "Define inline comment"},
            {"onEolCom", "Enable end-of-line comments"},
            {"offEolCom", "Disable end-of-line comments"},
            {"eolCom", "Define end-of-line comments"},
            {"onMultiR", "Multiple data statements on - replacing existing data"},
            {"onMulti", "Multiple data statements on - merging into existing data"},
            {"offMulti", "Multiple data statements off"},
            {"onWarning", "Make errors 115, 116, 170, 171 into warnings"},
            {"offWarning", "Do not convert errors into warnings"},
            {"onSparse", "Allow unsave calculations"},
            {"offSparse", "Do not allow unsave calculations"},
            {"stars", "Define the **** symbol"},
            {"onMixed", "Mixed operation on"},
            {"offMixed", "Mixed operations off"},
            {"onRecurse", "Enable recursive include files"},
            {"offRecurse", "Disable recursive include files"},
            {"onUni", "Allow assignments to predefined symbols"},
            {"offUni", "Do not allow assignments to predefined symbols"},
            {"onEps", "Zeros in data statements are entered as EPS"},
            {"offEps", "Zeros in data statements are entered as zeros"},
            {"onDelim", "Delimited data statement syntax on"},
            {"offDelim", "Delimited data statement syntax off"},
            {"onEmpty", "Allow empty data statements"},
            {"offEmpty", "Do not allow empty data statements"},
            {"onStrictSingleton", "Error if assignment to singleton set has multiple elements"},
            {"offStrictSingleton", "Take first record if assignment to singleton set has multiple elements"},
            {"onEnd", "END syntax on"},
            {"offEnd", "END syntax off"},
            {"onListing", "Do not list input lines"},
            {"offListing", "Resume listing input lines"},
            {"error", "Issue an error message"},
            {"abort", "Issue an error message and abort"},
            {"kill", "Removes all data and resets object to uninitialized"},
            {"clear", "Reset objects to all default values"},
            {"slice", "Clears parts of a data structure"},
            {"offLog", "Turn off line logging"},
            {"onLog", "Reset line logging"},
            {"onTroll", "Recognize Troll periodicity in id*id set elements"},
            {"offTroll", "Do not recognize Troll periodicity"},
            {"onOrder", "Lag and lead operations on constant and ordered sets only"},
            {"offOrder", "allow lag and lead operations on dynamic or unordered sets"},
            {"embeddedCode", "Start of block embedded code with substitution (execution time)"},
            {"embeddedCodeS", "Start of block embedded code with substitution (execution time)"},
            {"embeddedCodeV", "Start of block embedded code without substitution (execution time)"},
            {"pauseEmbeddedCode", "Pause of block embedded code (execution time)"},
            {"continueEmbeddedCode", "Continue of paused block embedded code with substitution (execution time)"},
            {"continueEmbeddedCodeS", "Continue of paused block embedded code with substitution (execution time)"},
            {"continueEmbeddedCodeV", "Continue of paused block embedded code without substitution (execution time)"},
            {"endEmbeddedCode", "End of block embedded code (execution time)"},
            {"onEmbeddedCode", "Start of block embedded code with substitution"},
            {"onEmbeddedCodeS", "Start of block embedded code with substitution"},
            {"onEmbeddedCodeV", "Start of block embedded code without substitution"},
            {"offEmbeddedCode", "End of block embedded code"},
            {"embeddedFree", "Frees and unloads an embedded code library"},
            {"gdxIn", "Open GDX file for input"},
            {"gdxOut", "Open GDX file for output"},
            {"load", "Load symbols from GDX file - domain filtered"},
            {"loadDc", "Load symbols from GDX file - domain checked"},
            {"loadR", "Load symbols from GDX file - domain filtered - replace"},
            {"loadM", "Load symbols from GDX file - domain filtered - merge"},
            {"loadDcR", "Load symbols from GDX file - domain checked - replace"},
            {"loadDcM", "Load symbols from GDX file - domain checked - merge"},
            {"loadIdx", "Load index parameters from GDX with implicit loading of domain - domain checked"},
            {"unload", "Unload symbols into GDX file"},
            {"setLocal", "Define a local environment variable"},
            {"setGlobal", "Define a global environment variable"},
            {"set", "Define a scoped environment variable"},
            {"evalLocal", "Evaluates and define a local environment variable"},
            {"evalGlobal", "Evaluates and define a global environment variable"},
            {"eval", "Evaluates and define a scoped environment variable"},
            {"setEnv", "Define an OS system environment"},
            {"dropLocal", "Drop a local environment variable"},
            {"dropGlobal", "Drop a global environment variable"},
            {"drop", "Drop a local environment variable"},
            {"dropEnv", "Drop an OS system environment variable"},
            {"setArgs", "Define local environment variables using argument list"},
            {"setNames", "Unpack a filename into local environment variables"},
            {"splitOption", "Split an option value pair string into environment variables"},
            {"setComps", "Unpack dotted names into local variables"},
            {"escape", "Define the % escape symbol"},
            {"onGlobal", "Allow global setting of $options"},
            {"offGlobal", "Do not allow global setting of $options"},
            {"setDdList", "Check double dash GAMS parameters"},
            {"prefixPath", "Prefix the path environment variable"},
            {"show", "Show current GAMS environment variables"},
            {"macro", "Preprocessing macro definition"},
            {"onMacro", "Recognize macros for expansion"},
            {"offMacro", "Do not recognize macros for expansion"},
            {"onDotL", "Assume .l for variables in assignments"},
            {"offDotL", "Do not assume .l for variables in assignments"},
            {"onExpand", "Expand macros when processing macro arguments"},
            {"offExpand", "Do not expand macros when processing macro arguments"},
            {"offLocal", "Limit .local nesting to one"},
            {"onLocal", "No limit on .local nesting"},
            {"onDotScale", "Assume .scale for var and equ references"},
            {"offDotScale", "do not assume .scale for var and equ references"},
            {"expose", "Remove all access control restrictions"},
            {"protect", "Protect objects from user modification"},
            {"hide", "Hide objects from user"},
            {"purge", "Remove the objects and all associated data"},
            {"compress", "Create compressed GAMS system file"},
            {"encrypt", "Create encrypted GAMS system file"},
            {"decompress", "Decompresses a GAMS system file"},
            {"run", "Not implemented"},
            {"stop", "Stops compilation"},
            {"exit", "Jumps to the end of the current file"},
            {"log", "Send message to the log"},
            {"echo", "Echo a string to a file"},
            {"echoN", "Echo a string to a file without ending the line"},
            {"use205", "Use GAMS version 205 syntax"},
            {"use225", "Use GAMS version 225 syntax"},
            {"use999", "Use current GAMS syntax"},
            {"diff", "Not Implemented"},
            {"onEcho", "Start of block echo with substitution"},
            {"onEchoV", "Start of block echo without substitution"},
            {"onEchoS", "Start of block echo with substitution"},
            {"offEcho", "End of block echo"},
            {"onPut", "Start of block put without substitution"},
            {"onPutV", "Start of block put without substitution"},
            {"onPutS", "Start of block put with substitution"},
            {"offPut", "End of block put"},
            {"clearError", "Clear compilation error"},
            {"clearErrors", "Clear compilation errors"},
            {"warning", "Issue compilation warning"},
            {"terminate", "Terminate compilation and execution"},
            {"remark", "Comment line with suppressed line number"},
            {"onUndf", "Allow UNDF as input"},
            {"offUndf", "Do not allow UNDF as input"},
            {"onEmbedded", "Allow embedded text or data in set and parameter statements"},
            {"offEmbedded", "No embedded test or data allowed"},
            {"onVerbatim", "Start verbatim copy if dumpopt > 9"},
            {"offVerbatim", "Stop verbatim copy"},
            {"ifThen", "If then elseif structure with case sensitive compare"},
            {"ifThenI", "If then elseif structure with case insensitive compare"},
            {"ifThenE", "If then elseif structure with expression evaluation"},
            {"elseIf", "Else if clause structure with case sensitive compare"},
            {"elseIfI", "Else if clause structure with case insensitive compare"},
            {"elseIfE", "Else if clause structure with expression evaluation"},
            {"else", "Else clause"},
            {"endIf", "Closing of IfThen control structure"},
            {"eject", "Start a new page in listing file"},
        };
        return list;
    }

    /* Generated by options.gms */
    static QList<QPair<QString, QString>> options() {
        QList<QPair<QString, QString>> list = {
            {"LP", "Linear Programming - default solver"},
            {"MIP", "Mixed-Integer Programming - default solver"},
            {"RMIP", "Relaxed Mixed-Integer Programming - default solver"},
            {"NLP", "Non-Linear Programming - default solver"},
            {"MCP", "Mixed Complementarity Problems - default solver"},
            {"MPEC", "Mathematical Programs with Equilibrium Constraints - default solver"},
            {"RMPEC", "Relaxed Mathematical Programs with Equilibrium Constraints - default solver"},
            {"CNS", "Constrained Nonlinear Systems - default solver"},
            {"DNLP", "Non-Linear Programming with Discontinuous Derivatives - default solver"},
            {"RMINLP", "Relaxed Mixed-Integer Non-Linear Programming - default solver"},
            {"MINLP", "Mixed-Integer Non-Linear Programming - default solver"},
            {"QCP", "Quadratically Constrained Programs - default solver"},
            {"MIQCP", "Mixed Integer Quadratically Constrained Programs - default solver"},
            {"RMIQCP", "Relaxed Mixed Integer Quadratically Constrained Programs - default solver"},
            {"EMP", "Extended Mathematical Programs - default solver"},
            {"IntVarUp", "Set default upper bound on integer variables"},
            {"Profile", "Execution profiling"},
            {"LimRow", "Maximum number of rows listed in one equation block"},
            {"LimCol", "Maximum number of columns listed in one variable block"},
            {"IterLim", "Iteration limit of solver"},
            {"DomLim", "Domain violation limit solver default"},
            {"ResLim", "Wall-clock time limit for solver"},
            {"OptCR", "Relative Optimality criterion solver default"},
            {"OptCA", "Absolute Optimality criterion solver default"},
            {"SysOut", "Solver Status file reporting option"},
            {"SolPrint", "Solution report print option"},
            {"Bratio", "Basis acceptance threshold"},
            {"ForLim", "GAMS looping limit"},
            {"Seed", "Random number seed"},
            {"SavePoint", "Save solver point in GDX file"},
            {"SolveLink", "Solver link option"},
            {"Sys10", "Changes rpower to ipower when the exponent is constant and within 1e-12 of an integer"},
            {"Sys11", "Dynamic resorting if indices in assignment/data statements are not in natural order"},
            {"Sys12", "Pass model with generation errors to solver"},
            {"ProfileTol", "Minimum time a statement must use to appear in profile generated output"},
            {"Integer1", "Integer communication cell N"},
            {"Integer2", "Integer communication cell N"},
            {"Integer3", "Integer communication cell N"},
            {"Integer4", "Integer communication cell N"},
            {"Integer5", "Integer communication cell N"},
            {"Threads", "Number of threads to be used by a solver"},
            {"gdxUels", "Unload labels or UELs to GDX either squeezed or full"},
            {"strictSingleton", "Error if assignment to singleton set has multiple elements"},
            {"Sys13", "Backward compatible mathnew"},
            {"FDDelta", "Step size for finite differences"},
            {"FDOpt", "Options for finite differences"},
            {"Solver", "Default solver for all model types that the solver is capable to process"},
            {"SparseRun", "Switch between sparse and dense execution"},
            {"ThreadsAsync", "Number of threads to be used for asynchronous solve (solveLink=6)"},
            {"MCPRHoldfx", "Print list of rows that are perpendicular to variables removed due to the holdfixed setting"},
            {"AsyncSolLst", "Print solution listing when asynchronous solve (Grid or Threads) is used"},
            {"Decimals", "Decimal places for display statements"},
            {"DispNr", ""},
            {"DispWidth", "Number of characters to be printed in the column labels of all subsequent display statements"},
            {"DmpIns", "Causes GAMS to generate a list of the instructions in the LST file"},
            {"DmpSym", "Debugging option: causes GAMS to echo the symbol table to the listing file"},
            {"DmpOpt", "Debugging option: causes GAMS to echo the runtime option settings"},
            {"DualCheck", "Output on the reduced cost condition"},
            {"Eject", "Inject a page break into the LST file"},
            {"Measure", "Output of time and memory use since the last measure statement or the program beginning"},
            {"MemoryStat", "Show memory statistics in the LST file"},
            {"OldName", "Only allow 10 character item set element names for compatibility with systems that do not accept longer names"},
            {"PipeStat", "Pipe statistics (deprecated)"},
            {"Real1", "Real communication cell N"},
            {"Real2", "Real communication cell N"},
            {"Real3", "Real communication cell N"},
            {"Real4", "Real communication cell N"},
            {"Real5", "Real communication cell N"},
            {"Reform", "Reformulation level"},
            {"SolSlack", "Causes the equation output in the listing file to contain slack variable values instead of level values"},
            {"SolveOpt", "Multiple solve management"},
            {"SparseOpt", ""},
            {"SparseVal", "Interpret special values differently"},
            {"SubSystems", "Lists all solvers available as well as the current default and active solvers in the LST file"},
            {"Sys0", "Tracing of interpreter"},
            {"Sys1", "Tracing of prescan"},
            {"Sys2", "Trace partial derivative generation"},
            {"Sys3", "Tracing nl code generation"},
            {"Sys4", "NL code statistics"},
            {"Sys5", "Debugging option for solvers"},
            {"Sys6", ""},
            {"Sys7", "Memory management statistics during model generation"},
            {"Sys8", "Pipe processing trace"},
            {"Sys9", "Procedure trace"},
            {"Work", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> reserved() {
        QList<QPair<QString, QString>> list = {
            {"abort", ""},
            {"Acronym", ""},
            {"Acronyms", ""},
            {"Alias", ""},
            {"all", ""},
            {"and", ""},
            {"break", ""},
            {"card", ""},
            {"continue", ""},
            {"display", ""},
            {"do", ""},
            {"else", ""},
            {"elseif", ""},
            {"endfor", ""},
            {"endif", ""},
            {"endloop", ""},
            {"endwhile", ""},
            {"eps", ""},
            {"execute", ""},
            {"execute_load", ""},
            {"execute_loaddc", ""},
            {"execute_loadhandle", ""},
            {"execute_loadpoint", ""},
            {"execute_unload", ""},
            {"execute_unloaddi", ""},
            {"execute_unloadidx", ""},
            {"for", ""},
            {"if", ""},
            {"inf", ""},
            {"logic", ""},
            {"loop", ""},
            {"na", ""},
            {"no", ""},
            {"not", ""},
            {"option", ""},
            {"options", ""},
            {"ord", ""},
            {"or", ""},
            {"ordascii", ""},
            {"ordebcdic", ""},
            {"procedure", ""},
            {"procedures", ""},
            {"prod", ""},
            {"put", ""},
            {"put_utilities", ""},
            {"put_utility", ""},
            {"putclear", ""},
            {"putclose", ""},
            {"putfmcl", ""},
            {"puthd", ""},
            {"putheader", ""},
            {"putpage", ""},
            {"puttitle", ""},
            {"puttl", ""},
            {"repeat", ""},
            {"smax", ""},
            {"smin", ""},
//            {"solve", ""},
            {"sum", ""},
            {"system", ""},
            {"then", ""},
            {"undf", ""},
            {"until", ""},
            {"while", ""},
            {"xor", ""},
            {"yes", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> embedded() {
        QList<QPair<QString, QString>> list = {
            {"embeddedCode", ""},
            {"embeddedCodeS", ""},
            {"embeddedCodeV", ""},
            {"continueEmbeddedCode", ""},
            {"continueEmbeddedCodeS", ""},
            {"continueEmbeddedCodeV", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> embeddedEnd() {
        QList<QPair<QString, QString>> list = {
            {"endEmbeddedCode", ""},
            {"pauseEmbeddedCode", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> declaration4Set() {
        QList<QPair<QString, QString>> list = {
            {"singleton", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> declaration4Var() {
        QList<QPair<QString, QString>> list = {
            {"binary", ""},
            {"free", ""},
            {"integer", ""},
            {"negative", ""},
            {"nonnegative", ""},
            {"positive", ""},
            {"semicont", ""},
            {"semiint", ""},
            {"sos1", ""},
            {"sos2", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> declaration() {
        QList<QPair<QString, QString>> list = {
            {"Equation", ""},
            {"Equations", ""},
            {"File", ""},
            {"Files", ""},
            {"Function", ""},
            {"Functions", ""},
            {"Model", ""},
            {"Models", ""},
            {"Parameter", ""},
            {"Parameters", ""},
            {"Scalar", ""},
            {"Scalars", ""},
            {"Set", ""},
            {"Sets", ""},
            {"Variable", ""},
            {"Variables", ""},
        };
        return list;
    }

    static QList<QPair<QString, QString>> modelType() {
        QList<QPair<QString, QString>> list = {
            {"LP", "Linear Program"},
            {"NLP", "Nonlinear Program"},
            {"QCP", "Quadratically Constrained Program"},
            {"DNLP", "Discontinuous Nonlinear Program"},
            {"MIP", "Mixed Integer Program"},
            {"RMIP", "Relaxed Mixed Integer Program"},
            {"RMINLP", "Relaxed Mixed Integer Nonlinear Program"},
            {"MINLP", "Mixed Integer Nonlinear Program"},
            {"MIQCP", "Mixed Integer Quadratically Constrained Program"},
            {"RMIQCP", "Relaxed Mixed Integer Quadratically Constrained Program"},
            {"MCP", "Mixed Complementarity Program"},
            {"CNS", "Constrained Nonlinear System"},
            {"MPEC", "Mathematical Programs with Equilibrium Constraints"},
            {"RMPEC", "Relaxed Mathematical Program with Equilibrium Constraints"},
            {"EMP", "Extended Mathematical Program"},
            {"MPSGE", "Extended Mathematical Program"},
            {"min", "minimize"},
            {"max", "maximize"},
            {"us", "using"},
        };
        return list;
    }

    static QList<QPair<QString, QString>> declarationTable() {
        QList<QPair<QString, QString>> list = {
            {"Table", ""},
        };
        return list;
    }

};


} // namespace syntax
} // namespace studio
} // namespace gans

#endif // SYNTAXDATA_H
