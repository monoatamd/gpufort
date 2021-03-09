def replaceAll(fSnippet,ppexpression,repl=lambda parseResult: ("",False),stripSearchString=True):
    """
    Replaces all matches for the given pyparsing expression with
    the string generated by the 'repl' function argument, 
    which takes the pyparsing parse result into account.
    """
    matched     = True
    transformed = False
    usedList    = []
    while matched:
        matched = False
        for tokens,begin,end in ppexpression.scanString(fSnippet):
             parseResult = tokens[0]
             subst, changed = repl(parseResult)
             transformed |= changed
             if not begin in usedList:
                 if changed:
                     searchString = fSnippet[begin:end]
                     if stripSearchString:
                         searchString = searchString.strip().strip("\n").strip()
                     fSnippet = fSnippet.replace(searchString,subst)
                 else:
                     usedList.append(begin) # do not check this match again
                 matched = True
                 break
    return fSnippet, transformed
def replaceFirst(fSnippet,ppexpression,repl=lambda parseResult: ("",False),stripSearchString=True):
    """
    Replaces the first match for the given pyparsing expression with
    the string generated by the 'repl' function argument, 
    which takes the pyparsing parse result into account.
    """
    transformed = False
    for tokens,begin,end in ppexpression.scanString(fSnippet):
        parseResult = tokens[0]
        subst, transformed = repl(parseResult)
        if transformed:
            searchString = fSnippet[begin:end]
            if stripSearchString:
                searchString = searchString.strip().strip("\n").strip()
            fSnippet = fSnippet.replace(searchString,subst)
        break
    return fSnippet, transformed

def eraseAll(fSnippet,ppexpression,stripSearchString=True):
    """
    Removes all matches for the given pyparsing expression
    """
    matched     = True
    transformed = False
    usedList    = []
    while matched:
        matched = False
        for tokens,begin,end in ppexpression.scanString(fSnippet):
            if not begin in usedList:
                searchString = fSnippet[begin:end]
                if stripSearchString:
                    searchString = searchString.strip().strip("\n").strip()
                fSnippet = fSnippet.replace(searchString,"")
                matched = True
                break
            transformed |= matched
    return fSnippet, transformed
def eraseFirst(fSnippet,ppexpression,stripSearchString=True):
    """
    Replaces the first match for the given pyparsing expression with
    the string generated by the 'repl' function argument, 
    which takes the pyparsing parse result into account.
    """
    transformed = False
    for tokens,begin,end in ppexpression.scanString(fSnippet):
        searchString = fSnippet[begin:end]
        if stripSearchString:
            searchString = searchString.strip().strip("\n").strip()
        fSnippet = fSnippet.replace(searchString,"")
        transformed = True
        break
    return fSnippet, transformed
