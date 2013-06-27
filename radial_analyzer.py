class Polynomial(object):
    """Polynomials, immutable, with floating point coefficients"""

    def __init__(self, c = 0, n = 0):
        self.__coeffs = [0 for i in range(n)] + [c]
        self.__standardize()

    def __standardize(self):
        while self.degree >= 0 and self.__coeffs[self.degree] == 0:
            self.__coeffs.pop()

    @property
    def degree(self):
        return len(self.__coeffs) - 1

    @property
    def constantTerm(self):
        if self.degree == -1:
            return 0
        else:
            return self.__coeffs[0]

    @property
    def leadingCoefficient(self):
        if self.degree == -1:
            return 0
        else:
            return self.__coeffs[self.degree]

    def __eq__(self, other):
        if self.degree != other.degree:
            return False
        for i in range(self.degree + 1):
            if self.__coeffs[i] != other.__coeffs[i]:
                return False
        return True
    def __ne__(self, other):
        return not (self == other)

    def __call__(self, x):
        total = 0
        for i in range(self.degree, -1, -1):
            total *= x
            total += self.__coeffs[i]
        return total

    def __add__(self, other):
        if isinstance(other, int) or isinstance(other, float):
            return self + Polynomial(other)
        if self.degree < other.degree:
            sm = self.__coeffs
            lg = other.__coeffs
        else:
            sm = other.__coeffs
            lg = self.__coeffs
        s = Polynomial()
        s.__coeffs = list(lg)
        for i in range(len(sm)):
            s.__coeffs[i] += sm[i]
        s.__standardize()
        return s

    def __radd__(self, other):
        return self + other

    def __pos__(self):
        return self

    def __neg__(self):
        n = Polynomial()
        n.__coeffs = [-x for x in self.__coeffs]
        return n

    def __sub__(self, other):
        return self + (-other)

    def __rsub__(self, other):
        return (-self) + other

    def __mul__(self, other):
        if isinstance(other, int) or isinstance(other, float):
            return self * Polynomial(other)
        p = Polynomial()
        p.__coeffs = [0] * (self.degree + other.degree + 1)
        for i in range(len(self.__coeffs)):
            for j in range(len(other.__coeffs)):
                p.__coeffs[i + j] += self.__coeffs[i] * other.__coeffs[j]
        p.__standardize()
        return p

    def __rmul__(self, other):
        return self * other

    def __truediv__(self, other):
        return self * (1 / other)

    def __pow__(self, e):
        if e < 0:
            raise ArithmeticError('Polynomial to a negative power')
        if e == 0:
            return Polynomial(1)
        if e == 1:
            return self
        if e % 2 == 0:
            return (self * self) ** (e >> 1)
        return self * (self ** (e - 1))

    def derivative(self):
        d = Polynomial()
        d.__coeffs = [i * self.__coeffs[i] for i in range(1, self.degree + 1)]
        return d


def factorial(n):
    if n < 0:
        raise ArithmeticError('Factorial of a negative number')
    f = 1
    for i in range(2, n + 1):
        f *= i
    return f


def choose(n, k):
    return factorial(n) // (factorial(k) * factorial(n - k))


def laguerre(n, a):
    x = Polynomial(1, 1)
    f = 0
    for i in range(n + 1):
        f += ((-1) ** i) * choose(n + a, n - i) * (x ** i) / factorial(i)
    return f


def bisect(f, lower, upper):
    if not (lower < upper):
        raise Exception('bisect: lower not less than upper')
    f_lower = f(lower)
    if f_lower == 0:
        return lower
    f_upper = f(upper)
    if f_upper == 0:
        return upper
    if (f_lower < 0 and f_upper < 0) or (f_lower > 0 and f_upper > 0):
        raise Exception('bisect: no sign change present')

    while True:
        mid = (lower + upper) / 2
        if not (lower < mid < upper):
            return mid
        f_mid = f(mid)
        if f_mid == 0:
            return mid
        if f_mid < 0:
            if f_lower < 0:
                lower = mid
                f_lower = f_mid
            else:
                upper = mid
                f_upper = f_mid
        else:
            if f_lower > 0:
                lower = mid
                f_lower = f_mid
            else:
                upper = mid
                f_upper = f_mid


def roots(f):
    if f.degree < 1:
        raise Exception('roots called on a constant polynomial')
    if f.degree == 1:
        return [-f.constantTerm / f.leadingCoefficient]
    df = f.derivative()
    df_roots = roots(df)

    leading_coeff_f = f.leadingCoefficient
    degree_f = f.degree

    # First, handle the case where df has no roots
    if len(df_roots) == 0:
        assert degree_f % 2 == 1
        f0 = f(0)
        if f0 == 0:
            return [0]
        if leading_coeff_f > 0 and f0 < 0:
            upper = 1
            while f(upper) <= 0:
                upper += 1
            return [bisect(f, 0, upper)]
        if leading_coeff_f > 0 and f0 > 0:
            lower = -1
            while f(lower) >= 0:
                lower -= 1
            return [bisect(f, lower, 0)]
        if leading_coeff_f < 0 and f0 > 0:
            upper = 1
            while f(upper) >= 0:
                upper += 1
            return [bisect(f, 0, upper)]
        if leading_coeff_f < 0 and f0 < 0:
            lower = -1
            while f(lower) <= 0:
                lower -= 1
            return [bisect(f, lower, 0)]
        raise Exception('Impossible monotonic polynomial')

    r = []

    # Check for a root to the left of the first root of df
    first_df_root = df_roots[0]
    f_at_first_df_root = f(first_df_root)
    negative_behavior_f = leading_coeff_f * ((-1) ** degree_f)
    if negative_behavior_f > 0 and f_at_first_df_root < 0:
        lower_bound_on_first_root = first_df_root - 1
        while f(lower_bound_on_first_root) <= 0:
            lower_bound_on_first_root -= 1
        r.append(bisect(f, lower_bound_on_first_root, first_df_root))
    if negative_behavior_f < 0 and f_at_first_df_root > 0:
        lower_bound_on_first_root = first_df_root - 1
        while f(lower_bound_on_first_root) >= 0:
            lower_bound_on_first_root -= 1
        r.append(bisect(f, lower_bound_on_first_root, first_df_root))

    # Look at each pair of roots of df
    for i in range(len(df_roots) - 1):
        dr1 = df_roots[i]
        dr2 = df_roots[i + 1]
        fdr1 = f(dr1)
        fdr2 = f(dr2)
        if fdr1 > 0 and fdr2 < 0 or fdr1 < 0 and fdr2 > 0:
            r.append(bisect(f, dr1, dr2))
        if fdr1 == 0:
            r.append(dr1)

    # Last one -- just check if it's a root of f
    if f(df_roots[-1]) == 0:
        r.append(df_roots[-1])

    # Check for a root to the right of the last root of df
    last_df_root = df_roots[-1]
    f_at_last_df_root = f(last_df_root)
    positive_behavior_f = leading_coeff_f
    if positive_behavior_f > 0 and f_at_last_df_root < 0:
        upper_bound_on_last_root = last_df_root + 1
        while f(upper_bound_on_last_root) <= 0:
            upper_bound_on_last_root += 1
        r.append(bisect(f, last_df_root, upper_bound_on_last_root))
    if positive_behavior_f < 0 and f_at_last_df_root > 0:
        upper_bound_on_last_root = last_df_root + 1
        while f(upper_bound_on_last_root) >= 0:
            upper_bound_on_last_root += 1
        r.append(bisect(f, last_df_root, upper_bound_on_last_root))

    return r


class Indenter(object):

    def __init__(self):
        self.__levels = []

    def indent(self, n):
        self.__levels.append(n)

    def dedent(self):
        self.__levels.pop()

    def out(self, str):
        return ' ' * sum(self.__levels) + str
