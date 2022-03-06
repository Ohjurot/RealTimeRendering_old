#pragma once
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

// Com pointer class
template<typename CT>
class ComPointer {
	public:
		// Default empty constructor
		ComPointer() = default;

		// Construct by raw pointer (add ref)
		ComPointer(CT* pointer) {
			// Clear internal pointer
			clearPointer();

			// Add ref and set
			setPointerAddRef(pointer);
		}

		// Construct by other compointer
		ComPointer(const ComPointer<CT>& other) {
			// Clear internal pointer
			clearPointer();

			// Set and add ref
			setPointerAddRef(other.m_pointer);
		}

		// Destructor
		~ComPointer() {
			// Clear pointer
			clearPointer();
		}

		// Release the pointer
		unsigned long release() {
			return clearPointer();
		}

		// Get reference (add ref)
		CT* getRef() {
			// Only when pointer exists
			if (m_pointer) {
				m_pointer->AddRef();
				return m_pointer;
			}

			// Fallback
			return nullptr;
		}

		// Get pointer (NO add ref)
		CT* get() {
			// Return pointer
			return m_pointer;
		}

		// Query interface function
		template<typename T>
		bool queryInterface(ComPointer<T>& other) {
			// Only on valid pointer
			if (m_pointer) {
				return m_pointer->QueryInterface(IID_PPV_ARGS(&other)) == S_OK;
			}

			// Fallback
			return false;
		}

		// Assign operator
		ComPointer<CT>& operator=(const ComPointer<CT>& other) {
			// Clear pointer
			clearPointer();

			// Set
			setPointerAddRef(other.m_pointer);
			return *this;
		}

		// Assign operator
		ComPointer<CT>& operator=(CT* other) {
			// Clear pointer
			clearPointer();

			// Set
			setPointerAddRef(other);
			return *this;
		}

		// Compare pointer
		bool operator==(const ComPointer<CT>& other) {
			return m_pointer == other.m_pointer;
		}

		// Compare pointer
		bool operator==(const CT* other) {
			return m_pointer == other;
		}

		// Pointer operator
		CT* operator->() {
			return m_pointer;
		}

		// Address of operator
		CT** operator&() {
			return &m_pointer;
		}

		// Auto conversion
		operator bool() {
			return m_pointer != nullptr;
		}

		// Implicit opperator CT
		operator CT* () {
			return m_pointer;
		}


	private:
		// Inline pointer clear function
		inline unsigned long clearPointer() {
			unsigned long newRef = 0;
			
			// Only on valid pointer
			if (m_pointer) {
				// Release and unset
				newRef = m_pointer->Release();
				m_pointer = nullptr;
			}

			return newRef;
		}

		// Inline set pointer and add ref call
		inline void setPointerAddRef(CT* pointer) {
			// Assign and add ref
			m_pointer = pointer;
			if (m_pointer) {
				m_pointer->AddRef();
			}
		}

	private:
		// Pointer container
		CT* m_pointer = nullptr;
};
