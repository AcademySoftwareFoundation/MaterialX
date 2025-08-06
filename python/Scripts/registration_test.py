#!/usr/bin/env python3
"""
Isolated test to find where the registration fails.
"""

try:
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
    print("* MaterialX imports successful")

    # Test 1: Create a minimal document loader
    print("\n=== Test 1: Create minimal loader ===")
    class TestLoader(mx_render.DocumentLoader):
        def __init__(self):
            print("Calling super().__init__...")
            super().__init__("test", "Test", "Test loader")
            print("* TestLoader.__init__ completed")
        
        def supportedExtensions(self):
            print("supportedExtensions() called")
            return {".test"}
        
        def importDocument(self, uri):
            print(f"importDocument({uri}) called")
            return mx.createDocument()
    
    print("Creating TestLoader instance...")
    loader = TestLoader()
    print("* TestLoader created successfully")
    
    # Test 2: Test each method individually
    print("\n=== Test 2: Test loader methods ===")
    print("Testing getIdentifier()...")
    identifier = loader.getIdentifier()
    print(f"* getIdentifier() = {identifier}")
    
    print("Testing getName()...")
    name = loader.getName()
    print(f"* getName() = {name}")
    
    print("Testing supportedExtensions()...")
    extensions = loader.supportedExtensions()
    print(f"* supportedExtensions() = {extensions}")
    
    # Test 3: Try registration
    print("\n=== Test 3: Registration test ===")
    print("About to call registerDocumentLoader()...")
    
    # Add a try-catch around just the registration
    try:
        result = mx_render.registerDocumentLoader(loader)
        print(f"* Registration completed with result: {result}")
    except Exception as e:
        print(f"✗ Registration failed with exception: {e}")
        import traceback
        traceback.print_exc()
    
    print("* Test completed successfully!")

except Exception as e:
    import traceback
    print(f"ERROR: {e}")
    traceback.print_exc()
