///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(__clang__)
#define GSL_SUPPRESS(x) [[gsl::suppress("x")]]
#else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define GSL_SUPPRESS(x) [[gsl::suppress(x)]]
#else
#define GSL_SUPPRESS(x)
#endif // _MSC_VER
#endif // __clang__

namespace gsl {
	// narrow_cast(): a searchable way to do narrowing casts of values
	template <class T, class U>
	// clang-format off
	GSL_SUPPRESS(type.1) // NO-FORMAT: attribute
		// clang-format on
		constexpr T narrow_cast(U&& u) noexcept
	{
		return static_cast<T>(std::forward<U>(u));
	}

}