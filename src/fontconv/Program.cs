using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.IO;

namespace fontconv
{
  public struct LineArg
  {
    public string Name { get; set; }
    public int Value { get; set; }
    public string Text { get; set; }
  }

  public class LineDescription
  {
    public string Operation { get; set; }
    public Dictionary<string, LineArg> Args = new Dictionary<string, LineArg>();

    public static LineDescription Parse(string line)
    {
      // Trim all spaces first to be safe
      line = line.Trim();

      LineDescription result = new LineDescription();

      // Parse line operation
      int cursor = line.IndexOf(' ');

      if (cursor < 0)
        cursor = line.Length;

      result.Operation = line.Substring(0, cursor).ToLower();

      while (cursor < line.Length)
      {
        int eqPos = line.IndexOf('=', cursor);
        if (eqPos < 0)
          break;

        LineArg arg = new LineArg();
        arg.Name = line.Substring(cursor, eqPos - cursor).Trim().ToLower();

        if (line[eqPos + 1] == '"')
        {
          cursor = line.IndexOf('"', eqPos + 2);
          if (cursor < 0)
            throw new Exception("Parsing error!");

          ++eqPos;
        }
        else
        {
          cursor = line.IndexOf(' ', eqPos + 1);
          if (cursor < 0)
            cursor = line.Length;
        }

        arg.Text = line.Substring(eqPos + 1, cursor - eqPos - 1).Trim();

        int value = 0;
        if (int.TryParse(arg.Text, out value))
          arg.Value = value;

        result.Args.Add(arg.Name, arg);
        ++cursor;
      }

      return result;
    }
  }

  public struct CharInfo
  {
    public int ID { get; set; }
    public int X { get; set; }
    public int Y { get; set; }
    public int Width { get; set; }
    public int Height { get; set; }
    public int XOffset { get; set; }
    public int YOffset { get; set; }
    public int XAdvance { get; set; }

    public static CharInfo Parse(LineDescription lineDesc)
    {
      CharInfo result = new CharInfo();

      result.ID = lineDesc.Args["id"].Value;
      result.X = lineDesc.Args["x"].Value;
      result.Y = lineDesc.Args["y"].Value;
      result.Width = lineDesc.Args["width"].Value;
      result.Height = lineDesc.Args["height"].Value;
      result.XOffset = lineDesc.Args["xoffset"].Value;
      result.YOffset = lineDesc.Args["yoffset"].Value;
      result.XAdvance = lineDesc.Args["xadvance"].Value;

      return result;
    }

    public void Write(BinaryWriter bw)
    {
      bw.Write(ID);
      bw.Write((ushort)X);
      bw.Write((ushort)Y);
      bw.Write((byte)Width);
      bw.Write((byte)Height);
      bw.Write((sbyte)XOffset);
      bw.Write((sbyte)YOffset);
      bw.Write((sbyte)XAdvance);
    }
  }

  public struct KerningInfo
  {
    public int First { get; set; }
    public int Second { get; set; }
    public int Amount { get; set; }

    public static KerningInfo Parse(LineDescription lineDesc)
    {
      KerningInfo result = new KerningInfo();

      result.First = lineDesc.Args["first"].Value;
      result.Second = lineDesc.Args["second"].Value;
      result.Amount = lineDesc.Args["amount"].Value;

      return result;
    }

    public void Write(BinaryWriter bw)
    {
      bw.Write(First);
      bw.Write(Second);
      bw.Write((sbyte)Amount);
    }
  }
  
  public class FontDescription
  {
    public string Texture { get; private set; }

    public int LineHeight { get; private set; }

    public int Base { get; private set; }

    public List<CharInfo> Chars = new List<CharInfo>();

    public List<KerningInfo> Kernings = new List<KerningInfo>();

    public static FontDescription Parse(string fileName)
    {
      FontDescription result = new FontDescription();

      string[] lines = File.ReadAllLines(fileName);
      foreach (var line in lines)
      {
        var lineDesc = LineDescription.Parse(line);
        
        switch (lineDesc.Operation)
        {
          case "common":
            result.LineHeight = lineDesc.Args["lineheight"].Value;
            result.Base = lineDesc.Args["base"].Value;

            if (lineDesc.Args["pages"].Value != 1)
              throw new Exception("Invalid number of texture pages - only 1 page is supported!");

            break;

          case "page":
            result.Texture = lineDesc.Args["file"].Text;

            if (lineDesc.Args["id"].Value != 0)
              throw new Exception("Invalid page ID!");

            break;

          case "char":
            result.Chars.Add(CharInfo.Parse(lineDesc));
            break;

          case "kerning":
            result.Kernings.Add(KerningInfo.Parse(lineDesc));
            break;
        }
      }

      var root = Path.GetDirectoryName(fileName);
      result.Texture = Path.Combine(root, result.Texture);

      return result;
    }

    public void Write(BinaryWriter bw)
    {
      // Font height info
      bw.Write((byte)LineHeight);
      bw.Write((byte)Base);

      // Write character infos
      bw.Write(Chars.Count);
      foreach (var charInfo in Chars)
        charInfo.Write(bw);

      // Write kerning infos
      bw.Write(Kernings.Count);
      foreach (var kerInfo in Kernings)
        kerInfo.Write(bw);
    }
  }

  public class Program
  {
    public static int Main(string[] args)
    {
      Program p = new Program();

      p.Run(new string[] { "..\\..\\data\\fonts\\Default.fnt" });
      //p.Run(new string[] { "..\\..\\data\\fonts\\Mono.fnt" });

      return 0;
    }

    public void PrintHelp()
    {
      Console.WriteLine("fontconv - converts BMFont output to single Base64 stream");
      Console.WriteLine("Usage:");
      Console.WriteLine("       fontconv [filename]");
      Console.WriteLine();
    }

    public int Run(string[] args)
    {
      try
      {
        if (args.Length != 1)
        {
          PrintHelp();
          return -1;
        }

        if (!File.Exists(args[0]))
        {
          Console.Error.WriteLine("File does not exist: " + args[0]);
          return -1;
        }

        var fontDesc = FontDescription.Parse(args[0]);

        // Load bitmap
        Bitmap b = new Bitmap(fontDesc.Texture);
        byte[] bytes = new byte[b.Width * b.Height / 8];
        int byteIndex = 0;

        for (int y = 0; y < b.Height; ++y)
        {
          for (int x = 0; x < b.Width; x += 8, ++byteIndex)
          {
            bytes[byteIndex] = 0;

            for (int i = 0; i < 8; ++i)
            {
              Color p = b.GetPixel(x + i, y);
              if (p.R >= 128 && p.G >= 128 && p.B >= 128)
                bytes[byteIndex] |= (byte)(1 << i);
            }
          }
        }

        // Write all into memory stream
        MemoryStream memStream = new MemoryStream();
        BinaryWriter bw = new BinaryWriter(memStream);

        // Write bitmap texture
        bw.Write((short)b.Width);
        bw.Write((short)b.Height);
        bw.Write(bytes);

        // Write font descriptor
        fontDesc.Write(bw);

        // Get bytes
        var base64 = System.Convert.ToBase64String(memStream.GetBuffer());
        int base64Len = base64.Length;

        // Write final output C-source file
        string outFileName = args[0] + ".h";
        TextWriter tw = File.CreateText(outFileName);

        int step = 120;
        for (int i = 0; i < base64.Length; i += step)
        {
          string line = (i + step < base64.Length) ? base64.Substring(i, step) : base64.Substring(i);
          tw.WriteLine("\"" + line + "\"");
        }

        tw.Flush();
        tw.Close();
        Console.WriteLine("Created: " + outFileName);
      }
      catch (Exception ex)
      {
        Console.Error.WriteLine("Exception: " + ex.Message);
        return -1;
      }

      return 0;
    }
  }
}
