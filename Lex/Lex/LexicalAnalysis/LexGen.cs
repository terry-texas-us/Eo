using System;
using System.IO;
using System.Collections;
namespace Lex
{
	enum transitionState {NONE, LITCHAR, RANGE};
	enum entryTypes {CLOSURE = 1, ALT, OR};
	
	/// <summary>
	/// Lexical analyzer table generator					.
	/// </summary>
	class LexGen
	{
		const int ENDLIST = - 1;
		
		// representation of NFA states
		const int MAXNFA = 4096;
		public struct nfaStruct
		{
			public transitionState ts;
			public int iToken;
			public int cval1;
			public int cval2;
			public int epsset;
		};
		public struct entryStruct
		{
			public entryTypes type;
			public int startstate;
			public int endstate;
		};
		// storage list table
		const int MAXEPS = 4096;
		static int[] epstab = new int[MAXEPS + 1];
		static int epstabp; // next free
		
		// representation of DFA states
		const int MAXDFA = 512;
		const int MAXCHAR = 255; // character code
		static int[,] dfanext = new int[MAXDFA + 1, MAXCHAR + 1];
		static int[] dfatoken = new int[MAXDFA + 1];
		static int d; // actual number of DFA states
		
		const int MAXSTATE = 500;
		static int [] iBase = new int[MAXSTATE + 1];
		static int [] tokval = new int[MAXSTATE + 1];
		static int [] iDefault = new int[MAXSTATE + 1];
		
		const int MAXENTRY = 4000;
		static int [] next = new int[MAXENTRY + 1];
		static int [] check = new int[MAXENTRY + 1];

		/// <summary>LexGen produces a table used for lexical analysis.</summary>
		[STAThread]
		static void Main(string[] args)
		{
			int [] startstate = new int[256];
			nfaStruct[] nfa = new nfaStruct[MAXNFA + 1];
	
			StreamReader srReadLine = new StreamReader("D:\\Projects\\Eo\\Lex\\lex_regexp.dat", System.Text.Encoding.ASCII);
			srReadLine.BaseStream.Seek(0, SeekOrigin.Begin);
			
			Console.WriteLine("Constructing NFA...");

			int olds;
			int nstates;
			int nentries;
			int ival = 0;
			
			int i = 1;
			int s = 2;
			epstabp = 1;
			
			while (srReadLine.Peek() > -1) 
			{
				string line = srReadLine.ReadLine();
				Console.WriteLine(line);
				int lp = 0;
				while (lp < line.Length && line[lp] == ' ') lp++;
				
				if (lp == line.Length || line[lp] == '!')
					continue;
				else if (is_ival(ref ival, line, ref lp))
				{
					while (lp < line.Length && line[lp] == ' ') lp++;
				
					if (lp == line.Length || line[lp] != '=')
						Console.WriteLine("Missing assignment operator");
					else 
					{
						lp++;
						startstate[i++] = s;
						olds = s;
						mknfa(ref s, nfa, line, ref lp);
						if (s == olds)
							Console.WriteLine("No NFA states generated for the token {0}", ival);
						else
							nfa[s].iToken = ival;
						s++;
					}
				}
				else
					Console.WriteLine("Invalid token value");
			}
			srReadLine.Close();

			s--;
			Console.WriteLine("# NFA states = {0}", s);
	
			startstate[i] = ENDLIST;
			nfa[1].epsset = epstabp;
			bubblesort(ref startstate);
			ascopy(startstate, ref epstabp, ref epstab);
			
			// TBC put case conversion here

			Console.WriteLine("Converting NFA to DFA...");
			mkdfa(nfa);
			Console.WriteLine("Number of DFA states = {0}", d);

			if (dfatoken[1] != 0)
			{
				Console.WriteLine("Token value {0} requires no input", dfatoken[1]);
				Console.WriteLine("Process aborted due to DFA error");
				// how to exit
			}
			Console.WriteLine("Optimizing DFA...");
			mkrfa();
			Console.WriteLine("Number of optimized DFA states = {0}", d);
			
			Console.WriteLine("Choosing default states...");
			mkdflt();

			Console.WriteLine("Creating compacted table...");
			mklex(out nstates, out nentries);
			Console.WriteLine("Writing output...");
			lexdump(nstates, nentries);
		}

		/// <summary>
		/// Copies an integer array tp settab, and updates setp
		/// </summary>
		public static void ascopy(int[] a, ref int setp, ref int[] settab)
		{
			int i = 1;
			while(a[i] != ENDLIST) {settab[setp++] = a[i++];}
			settab[setp++] = ENDLIST;
		}

		/// <summary>
		/// Sorts an integer set termineated by ENDLIST.
		/// </summary>
		public static void bubblesort(ref int[] a)
		{
			int i = 1;
			while (i < a.Length && a[i] != ENDLIST) i++;
			i--;
			while (i >= 2)
			{
				for (int j = 1; j <= i - 1; j++)
				{
					if (a[j] > a[j + 1])
					{
						int k = a[j];
						a[j] = a[j + 1];
						a[j + 1] = k;
					}
				}
				i--;
			}
		}
		/// <summary>
		/// Finds set of states that can be reached on empty string
		/// </summary>
		public static void eps_closure(ref int[] clset, int s, nfaStruct[] nfa, int[] epstab)
		{
			Stack stack = new Stack();

			bool found;
			clset[1] = s;
			int j = 2;
			int i = nfa[s].epsset;
			while (epstab[i] != ENDLIST)
			{
				stack.Push(epstab[i]);
				clset[j++] = epstab[i++];
			}
			while (stack.Count > 0)
			{
				i = nfa[(int) stack.Pop()].epsset;
				while (epstab[i] != ENDLIST)
				{
					found = false;
					int k = 1;
					while (!found && k < j)
					{
						if (clset[k++] == epstab[i]) found = true;
					}
					if (!found)
					{
						stack.Push(epstab[i]);
						clset[j++] = epstab[i];
					}
					i++;
				}
			}
			clset[j] = ENDLIST;
		}

		/// <summary>
		/// Computes eps_closure for a set of states.
		/// </summary>
		public static void eps_union(ref int[] clset, ref int[] P, nfaStruct[] nfa, ref int[] epstab)
		{
			int [] tmpset1 = new int[256];
			int [] tmpset2 = new int[256];
			int i = 1;
			while (P[i] != ENDLIST) i++;
			clset[1] = ENDLIST;
			if (i != 1)
			{
				for (int k = 1; k <= i - 1; k++)
				{
					eps_closure(ref tmpset1, P[k], nfa, epstab);
					int j = 1;
					while (clset[j] != ENDLIST)
					{
						tmpset2[j] = clset[j];
						j++;
					}
					tmpset2[j] = ENDLIST;
					union(tmpset1, tmpset2, ref clset);
				}
			}
		}

		public static bool eqvarray(ref int[] a1, ref int[] a2, int n)
		{
			for (int i = 1; i <= n; i++)
			{
				if (a1[i] != a2[i]) return false;
			}
			return true;
		}

		/// <summary>
		/// Compares set with settab starting at P for equivalence.
		/// </summary>
		public static bool eqvset(int[] iSet, int p, int[] settab)
		{
			int i = 1;
			int j = p;

			bool equiv = true;
			while (equiv && iSet[i] != ENDLIST) 
			{
				equiv = iSet[i++] == settab[j++];
			}
			return equiv && (settab[j] == ENDLIST);
		}

		/// <summary>
		/// Interprets escaped characters
		/// </summary>
		public static char escchar(string line, ref int lp)
		{
			lp++;
			
			char c;
			switch (line[lp])
			{
				case 'n': c = '\n'; lp++; break;
				case 't': c = '\t'; lp++; break;
				case 'b': c = (char) 32; lp++; break;
				case '0': c = '\0'; lp++; break;
				case 'f': c = (char) 12; lp++; break;
				case '\\': c = (char) 26; lp++; break;
				case 'd':
					int iVal = int.Parse(line.Substring(lp + 1, 3));
					c = (char) iVal; lp += 4;					
					break;
				case 'h':
					iVal = int.Parse(line.Substring(lp + 1, 2), System.Globalization.NumberStyles.AllowHexSpecifier);
					c = (char) iVal; lp += 3;					
					break;
                		
				default:
					c = line[lp]; lp++; break;
			}
			return c;
		}
		public static bool is_element(int element, ref int[] set, int setp)
		{
			while (set[setp] != ENDLIST)
			{
				if (set[setp++] == element) return true;
			}
			return false;
		}

		/// <summary>
		/// Examines the start of a string to see if it contains an integer.
		/// </summary>
		/// <remarks>
		/// The line pointer skips spaces even if the first valid character
		///  is not a digit.
		/// </remarks>
		public static bool is_ival(ref int ival, string line, ref int lp)
		{
			while (line[lp] == ' ') lp++;
			
			int i = lp;
			if (line[i] == '+' || line[i] == '-') i++;
			
			bool bIsInt = Char.IsDigit(line, i) ? true : false; 
			if (bIsInt)
			{
				while (i < line.Length && Char.IsDigit(line, i)) i++;
				ival = Int32.Parse(line.Substring(lp, i - lp));
				lp = i;
			}
			return bIsInt;
		}

		public static void lexdump(int nstates, int nentries)
		{
			FileStream fs = new FileStream("D:\\Projects\\Eo\\Lex\\LexTable_.h", FileMode.Create, FileAccess.Write, FileShare.None);
 
			StreamWriter swLex = new StreamWriter(fs);
			
			swLex.WriteLine("static int iBase[] =");
			swLex.WriteLine("{");
			for (int i = 0; i <= nstates - 1; i++) {swLex.Write("{0},", iBase[i]);}
			swLex.WriteLine("{0}", iBase[nstates]);
			swLex.WriteLine("};");
			
			swLex.WriteLine("static int iDefault[] =");
			swLex.WriteLine("{");
			for (int i = 0; i <= nstates - 1; i++) {swLex.Write("{0},", iDefault[i]);}
			swLex.WriteLine("{0}", iDefault[nstates]);
			swLex.WriteLine("};");
			
			swLex.WriteLine("static int iTokVal[] =");
			swLex.WriteLine("{");
			for (int i = 0; i <= nstates - 1; i++) {swLex.Write("{0},", tokval[i]);}
			swLex.WriteLine("{0}", tokval[nstates]);
			swLex.WriteLine("};");
			
			swLex.WriteLine("static int iNext[] =");
			swLex.WriteLine("{");
			for (int i = 0; i < nentries - 1; i++) {swLex.Write("{0},", next[i]);}
			swLex.WriteLine("{0}", next[nentries - 1]);
			swLex.WriteLine("};");

			swLex.WriteLine("static int iCheck[] =");
			swLex.WriteLine("{");
			for (int i = 0; i < nentries - 1; i++) {swLex.Write("{0},", check[i]);}
			swLex.WriteLine("{0}", check[nentries - 1]);
			swLex.WriteLine("};");
			
			swLex.Flush();
			swLex.Close();
		}

		/// <summary>
		/// Contructs dfa from nfa
		/// 
		/// {M} - eps_closure of NFA state 1
		/// DFA state 1 = {M}
		/// push {M}
		/// do while (stack not empty)
		///  pop {M}
		///  do for each input char i
		///   {P} = set of states reachable for {M} on input i
		///   if ({P} is the empty set) then
		///    do nothing
		///   else
		///    {N} = eps_union({P})
		///    
		///    if ({N} already exists as a DFA state) then
		///     do nothing
		///    else
		///     push {N}
		///    end if
		///    add a transistion from {M} to {N} labeled i
		///   end if
		///  end do
		/// end do
		/// 
		/// </summary>
		public static void mkdfa(nfaStruct[] nfa)
		{
			int [] P = new int[16];
			int [] N = new int[64];
			
			eps_closure(ref N, 1, nfa, epstab);
			
			int [] subset = new int[MAXDFA + 1];
			int [] subtab = new int[8 * (MAXDFA + 1)];
			int subp = 1;
			subset[1] = subp;
			bubblesort(ref N);
			ascopy(N, ref subp, ref subtab);
			
			Stack stack = new Stack();
			stack.Push(1);
			
			d = 1;
			bool found;

			while (stack.Count > 0)
			{
				int M = (int) stack.Pop();
				for (int i = 0; i <= MAXCHAR; i++)
				{
					int j = subset[M];
					int k = 1;
					while (subtab[j] != ENDLIST)
					{
						if (nfa[subtab[j]].ts == transitionState.LITCHAR)
							found = nfa[subtab[j]].cval1 == i;
						else if (nfa[subtab[j]].ts == transitionState.RANGE)
							found = (i >= nfa[subtab[j]].cval1 && i <= nfa[subtab[j]].cval2);
						else
							found = false;
						
						if (found) {P[k++] = subtab[j] + 1;}
						j++;
					}
					P[k] = ENDLIST;
					if (k == 1)
					{
						continue;
					}
					else
					{
						eps_union(ref N, ref P, nfa, ref epstab);
						bubblesort(ref N);
						j = 0;
						found = false;
						while (!found && j < d)
						{
							j++;
							found = eqvset(N, subset[j], subtab);
						}
						if (found)
							dfanext[M, i] = j;
						else
						{
							d++;
							stack.Push(d);
							subset[d] = subp;
							bubblesort(ref N);
							ascopy(N, ref subp, ref subtab);
							dfanext[M, i] = d;
						}
					}
				}
			}
			for (int i = 1; i <= d; i++)
			{
				int j = subset[i];
				int smin = MAXNFA;
				int k = 0;
				while (subtab[j] != ENDLIST)
				{
					if (nfa[subtab[j]].iToken != 0)
					{
						if (subtab[j] < smin)
						{
							k = nfa[subtab[j]].iToken;
							smin = subtab[j];
						}
					}
					j++;
				}
				dfatoken[i] = k;
			}
		}

		public static void mkdflt()
		{
			const int MINTRANS = 4;
			bool [] used = new bool[MAXDFA + 1];
			bool [] changed = new bool[MAXDFA + 1];
			int [] ilist = new int[MAXCHAR + 2];
			int [] jlist = new int[MAXCHAR + 2];
			bool [] same = new bool[MAXCHAR + 1];

			ilist[1] = ENDLIST;
			jlist[1] = ENDLIST;

			for (int i = 1; i <= d; i++)
			{
				bool closure = true;
				int trcount = 0;
				for (int k = 0; k <= MAXCHAR; k++)
				{
					if (dfanext[i, k] != 0)
					{
						trcount++;
						if (dfanext[i, k] != i) {closure = false;}
					}
				}
				if (closure || (trcount <= MINTRANS))
				{
					used[i] = true;
					goto l1;
				}
				for (int j = 1; j <= d; j++)
				{
					if ((i != j) && !used[i] && !changed[j])
					{
						bool sameset = true;
						int k = 0;
						while ((k <= MAXCHAR) && sameset)
						{
							same[k] = (
								(dfanext[i, k] == 0 && dfanext[j, k] == 0) ||
								(dfanext[i, k] != 0 && dfanext[j, k] != 0));
							if (!same[k]) sameset = false;
							k++;
						}
						if (sameset)
						{
							int ii = 1;
							int jj = 1;
							for (k = 0; k <= MAXCHAR; k++)
							{
								same[k] = (dfanext[i, k] == dfanext[j, k]);
								if (!is_element(dfanext[i, k], ref ilist, 1))
								{
									ilist[ii++] = dfanext[i, k];
									ilist[ii] = ENDLIST;
								}
								if (!is_element(dfanext[j, k], ref jlist, 1))
								{
									jlist[jj++] = dfanext[j, k];
									jlist[jj] = ENDLIST;
								}
							}
							if (ii >= jj)
							{
								iDefault[i] = j;
								changed[i] = true;
								used[j] = true;
								for (k = 0; k <= MAXCHAR; k++)
								{
									if (same[k]) dfanext[i, k] = 0;
								}
								goto l1;
							}
						}
					}
				}
			l1:				continue;
			}
		}

		public static void mklex(out int nstates, out int nentries)
		{
			for (int i = 1; i <= MAXSTATE; i++) {iBase[i] = 0; tokval[i] = 0;}
			for (int i = 0; i <= MAXENTRY; i++) {next[i] = 0; check[i] = 0;}
			for (int i = 1; i <= d; i++) {tokval[i] = dfatoken[i];}

			//iDefault[1] = 0;
			for (int i = 0; i <= MAXCHAR; i++)
			{
				next[i] = dfanext[1, i];
				check[i] = (dfanext[1, i] == 0) ? 0 : 1;
			}
			int offset = 0;
			int freep = MAXCHAR + 1;
			for (int s = 2; s <= d; s++)
			{
				while (check[freep] != 0) freep++;
			
				int p = freep;
				bool done = false;
				while (!done)
				{
					offset = 0;
					while (offset <= MAXCHAR && dfanext[s, offset] == 0) {offset++;}
					iBase[s] = p - offset;
					bool conflict = false;
					for (int i = offset; i <= MAXCHAR; i++)
					{
						if (dfanext[s, i] != 0)
						{
							if (check[iBase[s] + i] != 0) conflict = true;
						}
					}
					if (conflict)
					{
						p++;
						while (check[p] != 0) p++;
					}
					else
					{
						done = true;
					}
				}
				for (int i = offset; i <= MAXCHAR; i++)
				{
					if (dfanext[s, i] != 0)
					{
						next[iBase[s] + i] = dfanext[s, i];
						check[iBase[s] + i] = s;
					}
				}
			}
			nstates = d;
			nentries = iBase[d] + MAXCHAR;
			int iUnused = 0;
			for (int s = 0; s <= nentries; s++) {if (check[s] == 0) iUnused++;}
			Console.WriteLine("Table entries (total/unused) = ({0}/{1})", nentries, iUnused);
		}

		/// <summary>
		/// Make an NFA from a regular expression.
		/// </summary>
		public static void mknfa(ref int s, nfaStruct[] nfa, string line, ref int lp)
		{
			char c;
			
			entryStruct[] entry = new entryStruct[32];
			int sp = 0;
			
			while (lp < line.Length && nextc(out c, line, ref lp) != '!')
			{
				if (c == '\\')
				{
					lp--;
					nfa[s].ts = transitionState.LITCHAR;
					nfa[s].cval1 = escchar(line, ref lp);
					nfa[s++].epsset = epstabp;
					epstab[epstabp++] = ENDLIST;
				}
				else if (c == '[')
				{
					nfa[s].ts = transitionState.RANGE;
					if (nextc(out c, line, ref lp) == '\\')
					{
						lp--;
						nfa[s].cval1 = escchar(line, ref lp);
					}
					else
						nfa[s].cval1 = c;
					if (nextc(out c, line, ref lp) != '-')
						Console.WriteLine("Invalid range syntax");
					if (nextc(out c, line, ref lp) == '\\')
					{
						lp--;
						nfa[s].cval2 = escchar(line, ref lp);
					}
					else
						nfa[s].cval2 = c;
					nfa[s++].epsset = epstabp;
					epstab[epstabp++] = ENDLIST;
					if (nextc(out c, line, ref lp) != ']')
						Console.WriteLine("Expecting ], encountered {0}.", c);
				}
				else if (c == '{')
				{
					entry[++sp].type = entryTypes.CLOSURE;
					entry[sp].startstate = s++;
				}
				else if (c == '}')
				{
					if (entry[sp].type != entryTypes.CLOSURE)
						Console.WriteLine("Closure delimiter mismatch");
					else
					{
						int [] tmpset = new int[4];
						nfa[s].epsset = epstabp;
						tmpset[1] = entry[sp].startstate + 1;
						tmpset[2] = s + 1;
						tmpset[3] = ENDLIST;
						bubblesort(ref tmpset);
						ascopy(tmpset, ref epstabp, ref epstab);
						nfa[s].ts = transitionState.NONE;
						
						tmpset[1] = s + 1;
						tmpset[2] = entry[sp].startstate + 1;
						tmpset[3] = ENDLIST;
						nfa[entry[sp].startstate].epsset = epstabp;
						bubblesort(ref tmpset);
						ascopy(tmpset, ref epstabp, ref epstab);
						nfa[entry[sp--].startstate].ts = transitionState.NONE;
						s++;
					}
				}
				else if (c == '(')
				{
					entry[++sp].type = entryTypes.ALT;
					entry[sp].startstate = s++;
					entry[++sp].type = entryTypes.OR;
					entry[sp].startstate = s;
				}
				else if (c == '|')
				{
					entry[sp].endstate = s++;
					entry[++sp].type = entryTypes.OR;
					entry[sp].startstate = s;
				}
				else if (c == ')')
				{
					int [] tmpset = new int[20];
					entry[sp].endstate = s++;
					int i = 1;
					bool done = false;
					while (!done)
					{
						if (entry[sp].type == entryTypes.OR)
						{
							tmpset[i++] = entry[sp].startstate;
							nfa[entry[sp].endstate].epsset = epstabp;
							epstab[epstabp++] = s;
							epstab[epstabp++] = ENDLIST;
							nfa[entry[sp--].endstate].ts = transitionState.NONE;
						}
						else if (entry[sp].type == entryTypes.ALT)
						{
							tmpset[i] = ENDLIST;
							nfa[entry[sp].startstate].epsset = epstabp;
							bubblesort(ref tmpset);
							ascopy(tmpset, ref epstabp, ref epstab);
							nfa[entry[sp--].startstate].ts = transitionState.NONE;
							done = true;
						}
						else
							Console.WriteLine("Or delimiter mismatch");
					}
				}
				else
				{
					nfa[s].ts = transitionState.LITCHAR;
					nfa[s].cval1 = c;
					nfa[s++].epsset = epstabp;
					epstab[epstabp++] = ENDLIST;
				}	
			}
			nfa[s].epsset = epstabp;
			epstab[epstabp++] = ENDLIST;
		}

		/// <summary>
		/// Determines membership in a set.
		/// </summary>
		public static void mkrfa()
		{
			int [] G = new int[MAXDFA + 1];
			int [] Gtoken = new int[MAXDFA + 1];
			int [] oldG = new int[MAXDFA + 1];
			int [,] nextG = new int[MAXDFA + 1, MAXCHAR + 1];
			int [] group = new int[MAXDFA + 1];
			bool same;
			bool found;
			int r = 1;
			G[0] = 0;
			for (int i = 1; i <= d; i++)
			{
				G[i] = 0;
				oldG[i] = 0;
			}
			for (int i = 1; i <= d; i++)
			{
				if (G[i] != 0)
				{
					continue;
				}
				else
				{
					if (dfatoken[i] == 0)
					{
						G[i] = 1;
					}
					else
					{
						r++;
						G[i] = r;
						for (int j = i; j <= d; j++)
						{
							if (dfatoken[i] == dfatoken[j])
							{
								G[j] = G[i];
							}
						}
					}
				}
			}
			while (!eqvarray(ref G, ref oldG, d))
			{
				for (int i = 1; i <= d; i++)
				{
					oldG[i] = G[i];
					for (int j = 0; j <= MAXCHAR; j++)
					{
						nextG[i, j] = G[dfanext[i, j]];
					}
				}
				for (int k = 1; k <= r; k++)
				{
					int n = 0;
					for (int j = 1; j <= d; j++)
					{
						if (G[j] == k) {n++; group[n] = j;}
					}
					for (int i = 2; i <= n; i++)
					{
						int j = 0;
						found = false;
						while ((j < i - 1) && (!found))
						{
							j++;
							same = true;
							int c = 0;
							while (c <= MAXCHAR && same)
							{
								if (nextG[group[j], c] != nextG[group[i], c]) {same = false;}
								c++;
							}
							if (same) found = true;
						}
						if (!found)
						{
							r++;
							G[group[i]] = r;
						}
						else
						{
							G[group[i]] = G[group[j]];
						}
					}
				}
			}
			for (int i = 1; i <= r; i++)
			{
				for (int j = 1; j <= d; j++)
				{
					if (G[j] == i)
					{
						Gtoken[i] = dfatoken[j];
						for (int k = 0; k <= MAXCHAR; k++)
						{
							dfanext[i, k] = nextG[j, k];
						}
					}
				}
			}
			for (int i = 1; i <= r; i++)
			{
				dfatoken[i] = Gtoken[i];
			}
			d = r;
		}

		/// <summary>
		/// Get next non-blank character from line.
		/// </summary>
		/// <remarks>
		/// The line pointer is left pointing just past the character returned.
		/// </remarks>
		public static char nextc(out char c, string line, ref int lp)
		{
			while (line[lp] == ' ') lp++;
			c = line[lp++];
			return c;
		}

		/// <summary>
		/// Finds the union of two integer sets.
		/// </summary>
		public static void union(int[] set1, int[] set2, ref int[] uset)
		{
			int i = 1;
			int j = 1;
			while (set1[i] != ENDLIST)
			{
				if (!is_element(set1[i], ref set2, 1))
				{
					uset[j] = set1[i];
					j++;
				}
				i++;
			}
			i = 1;
			while (set2[i] != ENDLIST)
			{
				uset[j] = set2[i];
				i++;
				j++;
			}
			uset[j] = ENDLIST;
		}

	}
}
