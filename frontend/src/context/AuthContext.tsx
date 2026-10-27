import React, { createContext, useContext, useState, useEffect } from 'react';
import { User } from '../types';
import { authApi } from '../services/api';

interface AuthContextType {
  user: User | null;
  token: string | null;
  isAuthenticated: boolean;
  isAdmin: boolean;
  login: (username: string, password: string) => Promise<void>;
  register: (data: { fullName: string; email: string; phone: string; username: string; password: string }) => Promise<void>;
  logout: () => void;
  loading: boolean;
}

const AuthContext = createContext<AuthContextType>({
  user: null,
  token: null,
  isAuthenticated: false,
  isAdmin: false,
  login: async () => {},
  register: async () => {},
  logout: () => {},
  loading: true,
});

export const AuthProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [user, setUser] = useState<User | null>(null);
  const [token, setToken] = useState<string | null>(localStorage.getItem('tradex_token'));
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    const savedUser = localStorage.getItem('tradex_user');
    if (token && savedUser) {
      try {
        setUser(JSON.parse(savedUser));
      } catch {
        localStorage.removeItem('tradex_user');
      }
    }
    setLoading(false);
  }, [token]);

  const login = async (username: string, password: string) => {
    const res = await authApi.login({ username, password });
    const payload = res.data?.data || res.data;
    if (payload?.token && payload?.user) {
      const newToken = payload.token;
      const newUser = payload.user;
      setToken(newToken);
      setUser(newUser);
      localStorage.setItem('tradex_token', newToken);
      localStorage.setItem('tradex_user', JSON.stringify(newUser));
    } else {
      throw new Error(res.data?.message || 'Invalid response from server');
    }
  };

  const register = async (data: { fullName: string; email: string; phone: string; username: string; password: string }) => {
    const res = await authApi.register(data);
    const payload = res.data?.data || res.data;
    if (payload?.token && payload?.user) {
      const newToken = payload.token;
      const newUser = payload.user;
      setToken(newToken);
      setUser(newUser);
      localStorage.setItem('tradex_token', newToken);
      localStorage.setItem('tradex_user', JSON.stringify(newUser));
    } else {
      throw new Error(res.data?.message || 'Registration failed');
    }
  };

  const logout = () => {
    setToken(null);
    setUser(null);
    localStorage.removeItem('tradex_token');
    localStorage.removeItem('tradex_user');
  };

  const isAdmin = user?.role === 'ADMIN' || user?.role === 'SUPER_ADMIN';

  return (
    <AuthContext.Provider
      value={{
        user,
        token,
        isAuthenticated: !!token,
        isAdmin,
        login,
        register,
        logout,
        loading,
      }}
    >
      {children}
    </AuthContext.Provider>
  );
};

export const useAuth = () => useContext(AuthContext);
