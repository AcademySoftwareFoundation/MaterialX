import MaterialX as mx
'''
    Base MaterialX utilities
    - version checking

    Requires: MaterialX package
'''
def haveVersion(major, minor, patch):
    '''
    Check if the current vesion matches a given version
    ''' 
    imajor, iminor, ipatch = mx.getVersionIntegers()

    if major >= imajor:
        if  major > imajor:
            return True        
        if iminor >= minor:
            if iminor > minor:
                return True 
            if  ipatch >= patch:
                return True
    return False
